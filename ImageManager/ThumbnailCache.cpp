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
#include <QBuffer>
#include <QCache>
#include <QTemporaryFile>
#include <QDir>
#include <Settings/SettingsData.h>
#include <QTimer>
#include <QPixmap>
#include <QFile>

// We split the thumbnails into chunks to avoid a huge file changing over and over again, with a bad hit for backups
const int MAXFILESIZE=32*1024*1024;
const int FILEVERSION=4;
// We map some thumbnail files into memory and manage them in a least-recently-used fashion
const size_t LRU_SIZE=2;

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
            qWarning("Failed to open thumbnail file");

        uchar * data = file.map( 0, file.size() );
        if ( !data || QFile::NoError != file.error() )
        {
            qWarning("Failed to map thumbnail file");
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
    : m_currentFile(0), m_currentOffset(0), m_unsaved(0)
{
    m_memcache = new QCache<int,ThumbnailMapping>(LRU_SIZE);
    const QString dir = thumbnailPath(QString());
    if ( !QFile::exists(dir) )
        QDir().mkpath(dir);

    load();
    m_timer = new QTimer;
    connect( m_timer, SIGNAL(timeout()), this, SLOT(save()));
}

ImageManager::ThumbnailCache::~ThumbnailCache()
{
    delete m_memcache;
}

void ImageManager::ThumbnailCache::insert( const DB::FileName& name, const QImage& image )
{
    QFile file( fileNameForIndex(m_currentFile) );
    if ( ! file.open(QIODevice::ReadWrite ) )
    {
        qWarning("Failed to open thumbnail file for inserting");
        return;
    }
    if ( ! file.seek( m_currentOffset ) )
    {
        qWarning("Failed to seek in thumbnail file");
        return;
    }

    // purge in-memory cache for the current file:
    m_memcache->remove( m_currentFile );
    QByteArray data;
    QBuffer buffer( &data );
    bool OK = buffer.open( QIODevice::WriteOnly );
    Q_ASSERT(OK); Q_UNUSED(OK);

    OK = image.save( &buffer, "JPG" );
    Q_ASSERT( OK );

    const int size = data.size();
    if ( ! ( file.write( data.data(), size ) == size && file.flush() ) )
    {
        qWarning("Failed to write image data to thumbnail file");
        return;
    }
    file.close();

    m_map.insert( name, CacheFileInfo( m_currentFile, m_currentOffset, size ) );

    // Update offset
    m_currentOffset += size;
    if ( m_currentOffset > MAXFILESIZE ) {
        m_currentFile++;
        m_currentOffset = 0;
    }

    if ( ++m_unsaved > 100 )
        save();
    m_timer->start(1000);
}

QString ImageManager::ThumbnailCache::fileNameForIndex( int index ) const
{
    return thumbnailPath(QString::fromLatin1("thumb-") + QString::number(index) );
}

QPixmap ImageManager::ThumbnailCache::lookup( const DB::FileName& name ) const
{

    CacheFileInfo info = m_map[name];

    ThumbnailMapping *t = m_memcache->object(info.fileIndex);
    if (!t || !t->isValid())
    {
        t = new ThumbnailMapping( fileNameForIndex( info.fileIndex ) );
        if (!t->isValid())
        {
            qWarning("Failed to map thumbnail file");
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

void ImageManager::ThumbnailCache::save() const
{
    m_timer->stop();
    m_unsaved = 0;

    QTemporaryFile file;
    if ( !file.open() ) {
        qWarning("Failed to create temporary file");
        return;
    }

    QDataStream stream(&file);
    stream << FILEVERSION
           << m_currentFile
           << m_currentOffset
           << m_map.count();

    for( QMap<DB::FileName,CacheFileInfo>::ConstIterator it = m_map.begin(); it != m_map.end(); ++it ) {
        const CacheFileInfo& cacheInfo = it.value();
        stream << it.key().relative()
               << cacheInfo.fileIndex
               << cacheInfo.offset
               << cacheInfo.size;
    }
    file.close();

    const QString realFileName = thumbnailPath(QString::fromLatin1("thumbnailindex"));
    QFile::remove( realFileName );
    if ( !file.copy( realFileName ) )
        qWarning("Failed to copy the temporary file %s to %s", qPrintable( file.fileName() ), qPrintable( realFileName ) );

    QFile realFile( realFileName );
    realFile.open( QIODevice::ReadOnly );
    realFile.setPermissions( QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::WriteGroup | QFile::ReadOther );
    realFile.close();
}

void ImageManager::ThumbnailCache::load()
{
    QFile file( thumbnailPath( QString::fromLatin1("thumbnailindex")) );
    if ( !file.exists() )
        return;

    file.open(QIODevice::ReadOnly);
    QDataStream stream(&file);
    int version;
    stream >> version;
    if ( version != FILEVERSION )
        return; //Discard cache

    int count;
    stream >> m_currentFile
           >> m_currentOffset
           >> count;

    for ( int i = 0; i < count; ++i ) {
        QString name;
        int fileIndex;
        int offset;
        int size;
        stream >> name
               >> fileIndex
               >> offset
               >> size;
        m_map.insert( DB::FileName::fromRelativePath(name), CacheFileInfo( fileIndex, offset, size ) );
    }
}

bool ImageManager::ThumbnailCache::contains( const DB::FileName& name ) const
{
    return m_map.contains(name);
}

QString ImageManager::ThumbnailCache::thumbnailPath(const QString& file) const
{
    // Making it static is just an optimization.
    static QString base = QDir(Settings::SettingsData::instance()->imageDirectory()).absoluteFilePath( QString::fromLatin1(".thumbnails/") );
    return  base + file;
}

ImageManager::ThumbnailCache* ImageManager::ThumbnailCache::instance()
{
    if (!s_instance)
        s_instance = new ThumbnailCache;
    return s_instance;
}

void ImageManager::ThumbnailCache::deleteInstance()
{
    delete s_instance;
    s_instance = nullptr;
}

void ImageManager::ThumbnailCache::flush()
{
    for ( int i = 0; i <= m_currentFile; ++i )
        QFile::remove( fileNameForIndex(i) );
    m_currentFile = 0;
    m_currentOffset = 0;
    m_map.clear();
    m_memcache->clear();
    save();
}

void ImageManager::ThumbnailCache::removeThumbnail( const DB::FileName& fileName )
{
    m_map.remove( fileName );
    save();
}
// vi:expandtab:tabstop=4 shiftwidth=4:
