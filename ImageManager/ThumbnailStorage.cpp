/* Copyright (C) 2008 Jesper K. Pedersen <blackie@kde.org>
                      Henner Zeller <h.zeller@acm.org>

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

#include "ThumbnailStorage.h"
#include <QImage>

#include <qbuffer.h>
#include <qdir.h>


/*
 * The default format for images. The 'ppm' format seems to be fastest to work
 * with, so this is what we use as default.
 * Note, that <http://jens.triq.net/thumbnail-spec/> would require to use 'png'.
 */
static const char *kDefaultImageFormat = "ppm";

ImageManager::FileThumbnailStorage::FileThumbnailStorage(const QString & imageFormat)
    : _imageFormat(!imageFormat.isEmpty() ? imageFormat : QString::fromLatin1(kDefaultImageFormat))
{
    QDir dir(QDir::homePath());
    dir.mkdir( QString::fromLatin1( ".thumbnails" ) );
    dir.cd( QString::fromLatin1( ".thumbnails" ) );
    dir.mkdir( QString::fromLatin1( "normal" ) );
    dir.mkdir( QString::fromLatin1( "large" ) );
}

QString ImageManager::FileThumbnailStorage::keyToPath(const QString& key)
{
    return  QString::fromLatin1( "%1/.thumbnails/%2.%3" )
        .arg(QDir::homePath()).arg(key).arg(_imageFormat);
}

bool ImageManager::FileThumbnailStorage::store(const QString& key, const QImage& image)
{
    QString path = keyToPath(key);

    /* To prevent a race condition where another thread reads data from
     * a file that is not fully written to yet, we save the thumbnail into a
     * temporary file. Without this fix, you'd get plenty of
     * "libpng error: Read Error" messages when running more ImageLoader
     * threads. */
    QString temporary = path + QString::fromLatin1(".tmp");
    bool ok = image.save( temporary, _imageFormat.toLatin1() );

    if (!ok) return false;

    /* We can't use QFile::rename() here as we really want to overwrite
     * target file (perhaps we have a hash collision, or maybe it's
     * outdated) */
    rename( temporary.toLocal8Bit().constData(),
            path.toLocal8Bit().constData() );

    {
        QMutexLocker l(&_cacheLock);
        _existenceCache.insert(key);
    }
    return true;
}

void ImageManager::FileThumbnailStorage::remove(const QString& key)
{
    QFile::remove( keyToPath(key) );
}

bool ImageManager::FileThumbnailStorage::retrieve(const QString& key, QImage* image)
{
    QString path = keyToPath(key);
    if ( QFile::exists( path ) ) {
        return image->load( path, _imageFormat.toLatin1() );
    }
    return false;
}

bool ImageManager::FileThumbnailStorage::exists(const QString& key)
{
    {
        QMutexLocker l(&_cacheLock);
        if (_existenceCache.contains(key))
            return true;
    }
    QString path = keyToPath(key);
    bool exists = QFile::exists( path );
    if (exists) {
        QMutexLocker l(&_cacheLock);
        _existenceCache.insert(key);
    }

    return exists;
}

#ifdef TESTING_MEMORY_THUMBNAIL_CACHING
ImageManager::MemoryThumbnailStorage::MemoryThumbnailStorage(const char *imageFormat)
    : _imageFormat(imageFormat != NULL ? imageFormat : kDefaultImageFormat)
{
    /* nop */
}

bool ImageManager::MemoryThumbnailStorage::store(const QString& key, const QImage& image)
{
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    const char *qual = getenv("KPA_THUMB_QUALITY");  // for testing.
    int quality = qual == NULL ? -1 : atoi(qual);
    image.save(&buffer,  _imageFormat, quality);
    _cache.insert(key, ba);
    kDebug() << "Store '" << key << "'; size=" << ba.size()/1024 << "k";
    return true;
}

bool ImageManager::MemoryThumbnailStorage::retrieve(const QString& key, QImage* image)
{
    ImageCache::const_iterator found = _cache.find(key);
    if (found == _cache.end())
        return false;
    bool ok = image->loadFromData(found.value(), _imageFormat);
    kDebug() << "Load '" << key << "'; size=" << found.value().size()/1024 << "k; ok=" << ok;
    return ok;
}

bool ImageManager::MemoryThumbnailStorage::exists(const QString& key)
{
    ImageCache::const_iterator found = _cache.find(key);
    return (found != _cache.end());
}

#endif /* TESTING_MEMORY_THUMBNAIL_CACHING */
