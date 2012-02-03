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
#include <QTemporaryFile>
#include <QMutexLocker>
#include <QDir>
#include <Settings/SettingsData.h>
#include <QTimer>
#include <QPixmap>
#include <QFile>

/* If we're not using GNU C, elide __attribute__ */
#ifndef __GNUC__
#  define  __attribute__(x)  /*NOTHING*/
#endif

// We split the thumbnails into chunks to avoid a huge file changing over and over again, with a bad hit for backups
const int MAXFILESIZE=32*1024*1024;
const int FILEVERSION=3;

ImageManager::ThumbnailCache* ImageManager::ThumbnailCache::m_instance = 0;

ImageManager::ThumbnailCache::ThumbnailCache()
    : m_currentFile(0), m_currentOffset(0), m_unsaved(0)
{
    const QString dir = thumbnailPath(QString());
    if ( !QFile::exists(dir) )
        QDir().mkpath(dir);

    load();
    m_timer = new QTimer;
    connect( m_timer, SIGNAL(timeout()), this, SLOT(save()));
}

void ImageManager::ThumbnailCache::insert( const QString& name, const QImage& image )
{
    QFile file( fileNameForIndex(m_currentFile) );
    file.open(QIODevice::ReadWrite );
    file.seek( m_currentOffset );

    QByteArray data;
    QBuffer buffer( &data );
	// suppress compiler warnings when OK is not used:
    bool OK __attribute__((unused));
	OK = buffer.open( QIODevice::WriteOnly );
    Q_ASSERT( OK );

    OK = image.save( &buffer, "JPG" );
    Q_ASSERT( OK );

    const int size = data.size();
    file.write( data.data(), size );
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

QPixmap ImageManager::ThumbnailCache::lookup( const QString& name ) const
{

    CacheFileInfo info = m_map[name];

    QFile file( fileNameForIndex( info.fileIndex ) );
    file.open( QIODevice::ReadOnly );

    const char* data = (const char*) file.map( info.offset, info.size );
    QByteArray array( data, info.size );
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
        qWarning("Failed to crate temporary file");
        return;
    }

    QDataStream stream(&file);
    stream << FILEVERSION
           << m_currentFile
           << m_currentOffset
           << m_map.count();

    for( QMap<QString,CacheFileInfo>::ConstIterator it = m_map.begin(); it != m_map.end(); ++it ) {
        const CacheFileInfo& cacheInfo = it.value();
        stream << it.key()
               << cacheInfo.fileIndex
               << cacheInfo.offset
               << cacheInfo.size;
    }
    file.close();

    const QString realFileName = thumbnailPath(QString::fromLatin1("thumbnailindex"));
    QFile::remove( realFileName );
    if ( !file.copy( realFileName ) )
        qWarning("Failed to copy the temporary file %s to %s", qPrintable( file.fileName() ), qPrintable( realFileName ) );

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
        m_map.insert( name, CacheFileInfo( fileIndex, offset, size ) );
    }
}

bool ImageManager::ThumbnailCache::contains( const QString& name ) const
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
    if (!m_instance)
        m_instance = new ThumbnailCache;
    return m_instance;
}

void ImageManager::ThumbnailCache::flush()
{
    for ( int i = 0; i <= m_currentFile; ++i )
        QFile::remove( fileNameForIndex(i) );
    m_currentFile = 0;
    m_currentOffset = 0;
    m_map.clear();
    save();
}

void ImageManager::ThumbnailCache::removeThumbnail( const QString& fileName )
{
    m_map.remove( fileName );
    save();
}
