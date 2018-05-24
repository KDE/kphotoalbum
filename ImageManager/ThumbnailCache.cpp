/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "ThumbnailCache.h"
#include "Logging.h"

#include <Settings/SettingsData.h>
#include <DB/ImageDB.h>
#include <DB/FastDir.h>

#include <QBuffer>
#include <QCache>
#include <QTemporaryFile>
#include <QDir>
#include <QTimer>
#include <QPixmap>
#include <QFile>
#include <QElapsedTimer>

// We split the thumbnails into chunks to avoid a huge file changing over and over again, with a bad hit for backups
const int MAXFILESIZE=32*1024*1024;
const int FILEVERSION=4;
// We map some thumbnail files into memory and manage them in a least-recently-used fashion
const size_t LRU_SIZE=2;

const int thumbnailCacheSaveIntervalMS = (5 * 1000);

namespace ImageManager {
/**
 * The ThumbnailMapping wraps the memory-mapped data of a QFile.
 * Upon initialization with a file name, the corresponding file is opened
 * and its contents mapped into memory (as a QByteArray).
 *
 * Deleting the ThumbnailMapping unmaps the memory and closes the file.
 */
class ThumbnailMapping
{
public:
    ThumbnailMapping(const QString &filename)
        : file(filename),map(nullptr)
    {
        if ( !file.open( QIODevice::ReadOnly ) )
            qCWarning(ImageManagerLog, "Failed to open thumbnail file");

        uchar * data = file.map( 0, file.size() );
        if ( !data || QFile::NoError != file.error() )
        {
            qCWarning(ImageManagerLog, "Failed to map thumbnail file");
        }
        else
        {
            map = QByteArray::fromRawData( reinterpret_cast<const char*>(data), file.size() );
        }
    }
    bool isValid()
    {
        return !map.isEmpty();
    }
    // we need to keep the file around to keep the data mapped:
    QFile file;
    QByteArray map;
};
}

ImageManager::ThumbnailCache* ImageManager::ThumbnailCache::s_instance = nullptr;

ImageManager::ThumbnailCache::ThumbnailCache()
  : m_currentFile(0),
    m_currentOffset(0),
    m_timer(new QTimer),
    m_needsFullSave(true),
    m_isDirty(false),
    m_memcache(new QCache<int,ThumbnailMapping>(LRU_SIZE)),
    m_currentWriter(nullptr)
{
    const QString dir = thumbnailPath(QString());
    if ( !QFile::exists(dir) )
        QDir().mkpath(dir);

    load();
    connect(this, &ImageManager::ThumbnailCache::doSave, this, &ImageManager::ThumbnailCache::saveImpl);
    connect(m_timer, &QTimer::timeout, this, &ImageManager::ThumbnailCache::saveImpl);
    m_timer->setInterval(thumbnailCacheSaveIntervalMS);
    m_timer->setSingleShot(true);
    m_timer->start(thumbnailCacheSaveIntervalMS);
}

ImageManager::ThumbnailCache::~ThumbnailCache()
{
    m_needsFullSave = true;
    saveInternal();
    delete m_memcache;
    delete m_timer;
}

void ImageManager::ThumbnailCache::insert( const DB::FileName& name, const QImage& image )
{
    m_thumbnailWriterLock.lock();
    if ( ! m_currentWriter ) {
        m_currentWriter = new QFile( fileNameForIndex(m_currentFile) );
        if ( ! m_currentWriter->open(QIODevice::ReadWrite ) ) {
            qCWarning(ImageManagerLog, "Failed to open thumbnail file for inserting");
            m_thumbnailWriterLock.unlock();
            return;
        }
    }
    if ( ! m_currentWriter->seek( m_currentOffset ) )
    {
        qCWarning(ImageManagerLog, "Failed to seek in thumbnail file");
        m_thumbnailWriterLock.unlock();
        return;
    }

    m_dataLock.lock();
    // purge in-memory cache for the current file:
    m_memcache->remove( m_currentFile );
    QByteArray data;
    QBuffer buffer( &data );
    bool OK = buffer.open( QIODevice::WriteOnly );
    Q_ASSERT(OK); Q_UNUSED(OK);

    OK = image.save( &buffer, "JPG" );
    Q_ASSERT( OK );

    const int size = data.size();
    if ( ! ( m_currentWriter->write( data.data(), size ) == size && m_currentWriter->flush() ) )
    {
        qCWarning(ImageManagerLog, "Failed to write image data to thumbnail file");
        return;
    }
    
    if ( m_currentOffset + size > MAXFILESIZE ) {
        m_currentWriter->close();
        m_currentWriter = nullptr;
    }
    m_thumbnailWriterLock.unlock();

    if ( m_hash.contains(name) ) {
        CacheFileInfo info = m_hash[name];
        if ( info.fileIndex == m_currentFile && info.offset == m_currentOffset &&
             info.size == size ) {
            qDebug() << "Found duplicate thumbnail " << name.relative() << "but no change in information";
            m_dataLock.unlock();
            return;
        } else {
            // File has moved; incremental save does no good.
            qDebug() << "Found duplicate thumbnail " << name.relative() << " at new location, need full save! ";
            m_saveLock.lock();
            m_needsFullSave = true;
            m_saveLock.unlock();
        }
    }

    m_hash.insert( name, CacheFileInfo( m_currentFile, m_currentOffset, size ) );
    m_isDirty = true;

    m_unsavedHash.insert( name, CacheFileInfo( m_currentFile, m_currentOffset, size ) );

    // Update offset
    m_currentOffset += size;
    if ( m_currentOffset > MAXFILESIZE ) {
        m_currentFile++;
        m_currentOffset = 0;
    }
    int unsaved = m_unsavedHash.count();
    m_dataLock.unlock();

    // Thumbnail building is a lot faster now.  Even on an HDD this corresponds to less
    // than 1 minute of work.
    //
    // We need to call the internal version that does not interact with the timer.
    // We can't simply signal from here because if we're in the middle of loading new
    // images the signal won't get invoked until we return to the main application loop.
    if ( unsaved >= 100 ) {
        saveInternal();
    }
}

QString ImageManager::ThumbnailCache::fileNameForIndex( int index, const QString dir ) const
{
    return thumbnailPath(QString::fromLatin1("thumb-") + QString::number(index), dir );
}

QPixmap ImageManager::ThumbnailCache::lookup( const DB::FileName& name ) const
{
    m_dataLock.lock();
    CacheFileInfo info = m_hash[name];
    m_dataLock.unlock();

    ThumbnailMapping *t = m_memcache->object(info.fileIndex);
    if (!t || !t->isValid())
    {
        t = new ThumbnailMapping( fileNameForIndex( info.fileIndex ) );
        if (!t->isValid())
        {
            qCWarning(ImageManagerLog, "Failed to map thumbnail file");
            return QPixmap();
        }
        m_memcache->insert(info.fileIndex,t);
    }
    QByteArray array( t->map.mid(info.offset , info.size ) );
    QBuffer buffer( &array );
    buffer.open( QIODevice::ReadOnly );
    QImage image;
    image.load( &buffer, "JPG");

    // Notice the above image is sharing the bits with the file, so I can't just return it as it then will be invalid when the file goes out of scope.
    // PENDING(blackie) Is that still true?
    return QPixmap::fromImage( image );
}

QByteArray ImageManager::ThumbnailCache::lookupRawData( const DB::FileName& name ) const
{
    m_dataLock.lock();
    CacheFileInfo info = m_hash[name];
    m_dataLock.unlock();

    ThumbnailMapping *t = m_memcache->object(info.fileIndex);
    if (!t || !t->isValid())
    {
        t = new ThumbnailMapping( fileNameForIndex( info.fileIndex ) );
        if (!t->isValid())
        {
            qCWarning(ImageManagerLog, "Failed to map thumbnail file");
            return QByteArray();
        }
        m_memcache->insert(info.fileIndex,t);
    }
    QByteArray array( t->map.mid(info.offset , info.size ) );
    return array;
}

void ImageManager::ThumbnailCache::saveFull() const
{
    // First ensure that any dirty thumbnails are written to disk
    m_thumbnailWriterLock.lock();
    if ( m_currentWriter ) {
        m_currentWriter->close();
        m_currentWriter = nullptr;
    }
    m_thumbnailWriterLock.unlock();
    m_dataLock.lock();
    if ( ! m_isDirty ) {
        m_dataLock.unlock();
        return;
    }
    QTemporaryFile file;
    if ( !file.open() ) {
        qCWarning(ImageManagerLog, "Failed to create temporary file");
        m_dataLock.unlock();
        return;
    }
    QHash<DB::FileName, CacheFileInfo> tempHash = m_hash;

    m_unsavedHash.clear();
    m_needsFullSave = false;
    // Clear the dirty flag early so that we can allow further work to proceed.
    // If the save fails, we'll set the dirty flag again.
    m_isDirty = false;
    m_dataLock.unlock();

    QDataStream stream(&file);
    stream << FILEVERSION
           << m_currentFile
           << m_currentOffset
           << m_hash.count();

    for( QHash<DB::FileName,CacheFileInfo>::ConstIterator it = tempHash.begin(); it != tempHash.end(); ++it ) {
        const CacheFileInfo& cacheInfo = it.value();
        stream << it.key().relative()
               << cacheInfo.fileIndex
               << cacheInfo.offset
               << cacheInfo.size;
    }
    file.close();

    const QString realFileName = thumbnailPath(QString::fromLatin1("thumbnailindex"));
    QFile::remove( realFileName );
    if ( !file.copy( realFileName ) ) {
        qCWarning(ImageManagerLog, "Failed to copy the temporary file %s to %s", qPrintable( file.fileName() ), qPrintable( realFileName ) );
        m_dataLock.lock();
        m_isDirty = true;
        m_needsFullSave = true;
        m_dataLock.unlock();
    } else {
        QFile realFile( realFileName );
        realFile.open( QIODevice::ReadOnly );
        realFile.setPermissions( QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::WriteGroup | QFile::ReadOther );
        realFile.close();
    }
}

// Incremental save does *not* clear the dirty flag.  We always want to do a full
// save eventually.
void ImageManager::ThumbnailCache::saveIncremental() const
{
    m_thumbnailWriterLock.lock();
    if ( m_currentWriter ) {
        m_currentWriter->close();
        m_currentWriter = nullptr;
    }
    m_thumbnailWriterLock.unlock();
    m_dataLock.lock();
    if ( m_unsavedHash.count() == 0 ) {
        m_dataLock.unlock();
        return;
    }
    QHash<DB::FileName, CacheFileInfo> tempUnsavedHash = m_unsavedHash;
    m_unsavedHash.clear();
    m_isDirty = true;
    m_dataLock.unlock();

    const QString realFileName = thumbnailPath(QString::fromLatin1("thumbnailindex"));
    QFile file( realFileName );
    if ( ! file.open( QIODevice::WriteOnly | QIODevice::Append ) ) {
        qCWarning(ImageManagerLog, "Failed to open thumbnail cache for appending");
        m_needsFullSave = true;
        return;
    }
    QDataStream stream(&file);
    for( QHash<DB::FileName,CacheFileInfo>::ConstIterator it = tempUnsavedHash.begin(); it != tempUnsavedHash.end(); ++it ) {
        const CacheFileInfo& cacheInfo = it.value();
        stream << it.key().relative()
               << cacheInfo.fileIndex
               << cacheInfo.offset
               << cacheInfo.size;
    }
    file.close();
}

void ImageManager::ThumbnailCache::saveInternal() const
{
    m_saveLock.lock();
    const QString realFileName = thumbnailPath(QString::fromLatin1("thumbnailindex"));
    // If something has asked for a full save, do it!
    if ( m_needsFullSave || ! QFile( realFileName ).exists() ) {
        saveFull();
    } else {
        saveIncremental();
    }
    m_saveLock.unlock();
}

void ImageManager::ThumbnailCache::saveImpl() const
{
    m_timer->stop();
    saveInternal();
    m_timer->setInterval(thumbnailCacheSaveIntervalMS);
    m_timer->setSingleShot(true);
    m_timer->start(thumbnailCacheSaveIntervalMS);
}

void ImageManager::ThumbnailCache::save() const
{
    m_saveLock.lock();
    m_needsFullSave = true;
    m_saveLock.unlock();
    emit doSave();
}

void ImageManager::ThumbnailCache::load()
{
    QFile file( thumbnailPath( QString::fromLatin1("thumbnailindex")) );
    if ( !file.exists() )
        return;

    QElapsedTimer timer;
    timer.start();
    file.open(QIODevice::ReadOnly);
    QDataStream stream(&file);
    int version;
    stream >> version;
    if ( version != FILEVERSION )
        return; //Discard cache

    // We can't allow anything to modify the structure while we're doing this.
    m_dataLock.lock();
    int count = 0;
    stream >> m_currentFile
           >> m_currentOffset
           >> count;

    while ( ! stream.atEnd() ) {
        QString name;
        int fileIndex;
        int offset;
        int size;
        stream >> name
               >> fileIndex
               >> offset
               >> size;

        m_hash.insert( DB::FileName::fromRelativePath(name), CacheFileInfo( fileIndex, offset, size ) );
        if ( fileIndex > m_currentFile ) {
            m_currentFile = fileIndex;
            m_currentOffset = offset + size;
        } else if (fileIndex == m_currentFile && offset + size > m_currentOffset) {
            m_currentOffset = offset + size;
        }
        if ( m_currentOffset > MAXFILESIZE ) {
            m_currentFile++;
            m_currentOffset = 0;
        }
        count++;
    }
    m_dataLock.unlock();
    qDebug() << "Loaded thumbnails in " << timer.elapsed() / 1000.0 << " seconds";
}

void ImageManager::ThumbnailCache::optimizeThumbnails()
{
    const QString tmpdir(QString::fromLatin1(".thumbnails.tmp/"));
    const QString dir = thumbnailPath(QString(), tmpdir);
    const QString index = thumbnailPath(QString::fromLatin1("thumbnailindex"), tmpdir);
    QElapsedTimer timer;

    timer.start();
    int currentInputFile = -1;
    if ( !QFile::exists(dir) )
        QDir().mkpath(dir);

    int maxFile = 0;
    int maxOffset = 0;
    for ( const DB::FileName& fileName : DB::ImageDB::instance()->images()) {
        const CacheFileInfo& cacheInfo = m_hash[fileName];
        if ( cacheInfo.fileIndex > maxFile ) {
            maxFile = cacheInfo.fileIndex;
            maxOffset = cacheInfo.offset + cacheInfo.size;
        } else if ( cacheInfo.fileIndex == maxFile && 
                    cacheInfo.offset >= maxOffset + cacheInfo.size ) {
            maxOffset = cacheInfo.offset + cacheInfo.size;
        }
    }

    int currentFile = 0;
    int currentOffset = 0;

    QFile *outFile = nullptr;
    
    QFile *inFile = nullptr;

    QFile indexFile( index );
    if ( ! indexFile.open(QIODevice::ReadWrite ) ) {
        qCWarning(ImageManagerLog, "Failed to open index file for writing");
        return;
    }

    QDataStream stream(&indexFile);
    stream << FILEVERSION
           << maxFile
           << maxOffset
           << DB::ImageDB::instance()->images().count();

    for ( const DB::FileName& fileName : DB::ImageDB::instance()->images()) {
        const CacheFileInfo& cacheInfo = m_hash[fileName];
        if ( cacheInfo.fileIndex != currentInputFile ) {
            if ( inFile ) {
                inFile->close();
                delete inFile;
            }
            currentInputFile = cacheInfo.fileIndex;
            inFile = new QFile( fileNameForIndex(currentInputFile) );
            if ( ! inFile->open(QIODevice::ReadOnly ) ) {
                qDebug() << "Failed to open thumbnail file " << fileNameForIndex(currentInputFile) << " for reading";
                return;
            }
        }
        if ( ! inFile->seek( cacheInfo.offset ) ) {
            qCWarning(ImageManagerLog, "Failed to seek in input thumbnail file");
            return;
        }

        QByteArray data( inFile->read( cacheInfo.size ) );
        if ( data.size() != cacheInfo.size ) {
            qCWarning(ImageManagerLog, "Incomplete size!");
            return;
        }
        if ( ! outFile ) {
            outFile = new QFile( fileNameForIndex( currentFile, tmpdir ) );
            if ( ! outFile->open(QIODevice::ReadWrite ) ) {
                qCWarning(ImageManagerLog, "Failed to open thumbnail file for inserting");
                return;
            }
        }

        if ( ! ( outFile->write(data.data(), data.size() ) ) ) {
            qCWarning(ImageManagerLog, "Failed to write image data to thumbnail file!");
            return;
        }
        stream << fileName.relative()
               << currentFile
               << currentOffset
               << data.size();
        currentOffset += data.size();
        if ( currentOffset > MAXFILESIZE ) {
            outFile->close();
            delete outFile;
            outFile = nullptr;
            currentFile++;
            currentOffset = 0;
        }
    }
    indexFile.close();
    if ( inFile ) {
        inFile->close();
        delete inFile;
    }
    if ( outFile ) {
        outFile->close();
        delete outFile;
    }
    qDebug() << "Optimized thumbnails in " << timer.elapsed() / 1000.0 << " seconds";
}

bool ImageManager::ThumbnailCache::contains( const DB::FileName& name ) const
{
    m_dataLock.lock();
    bool answer = m_hash.contains(name);
    m_dataLock.unlock();
    return answer;
}

QString ImageManager::ThumbnailCache::thumbnailPath(const QString& file, const QString dir) const
{
    QString base = QDir(Settings::SettingsData::instance()->imageDirectory()).absoluteFilePath( dir );
    return  base + file;
}

ImageManager::ThumbnailCache* ImageManager::ThumbnailCache::instance()
{
    if (!s_instance) {
        s_instance = new ThumbnailCache;
    }
    return s_instance;
}

void ImageManager::ThumbnailCache::deleteInstance()
{
    delete s_instance;
    s_instance = nullptr;
}

void ImageManager::ThumbnailCache::flush()
{
    m_dataLock.lock();
    for ( int i = 0; i <= m_currentFile; ++i )
        QFile::remove( fileNameForIndex(i) );
    m_currentFile = 0;
    m_currentOffset = 0;
    m_isDirty = true;
    m_hash.clear();
    m_unsavedHash.clear();
    m_memcache->clear();
    m_dataLock.unlock();
    save();
}

void ImageManager::ThumbnailCache::removeThumbnail( const DB::FileName& fileName )
{
    m_dataLock.lock();
    m_isDirty = true;
    m_hash.remove( fileName );
    m_dataLock.unlock();
    save();
}
void ImageManager::ThumbnailCache::removeThumbnails( const DB::FileNameList& files )
{
    m_dataLock.lock();
    m_isDirty = true;
    Q_FOREACH(const DB::FileName &fileName, files) {
        m_hash.remove( fileName );
    }
    m_dataLock.unlock();
    save();
}
// vi:expandtab:tabstop=4 shiftwidth=4:
