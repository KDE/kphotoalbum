/* Copyright (C) 2008 Henner Zeller <h.zeller@acm.org>

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

#ifndef THUMBNAIL_STORAGE_H
#define THUMBNAIL_STORAGE_H

#include <qmap.h>
#include <qstring.h>
#include <QMutex>
#include <QSet>
class QImage;

namespace ImageManager {

/**
 * Interface of a backend to store thumbnails.
 * Implementations of ThumbnailStorage must be thread save.
 */
class ThumbnailStorage {
public:
    virtual ~ThumbnailStorage() {}

    /** Store an image under the given key. Returns success. */
    virtual bool store(const QString& key, const QImage& image) = 0;

    /**
     * Tries to retrieve an image stored under the given key. On success,
     * returns 'true' and fills in the image.
     */
    virtual bool retrieve(const QString& key, QImage* image) = 0;

    /**
     * Remove an image under the given key from the storage.
     */
    virtual void remove(const QString& key) = 0;

    /**
     * Check if the thumbnail with the given key exists. Use if you don't
     * want to load the thumbnail but check if you would need go generate it.
     */
    virtual bool exists(const QString& key) = 0;
};


/**
 * Default implementation of the ThumbnailStorage. It stores images
 * in ~/.thumbnails/${key}.${imageFormat}.
 */
class FileThumbnailStorage : public ThumbnailStorage {
public:
    /**
     * construct a FileThumbnailStorage. Store the images in the given
     * "imageFormat", which can be 'png', 'ppm', 'jpg'. The 'png' format
     * would be the standard format to be compatible with other
     * applications (see <http://jens.triq.net/thumbnail-spec/>), but turns
     * out that the 'ppm' format seems to be much faster.
     * If "imageFormat" is NULL, we fall back to 'ppm' as default.
     */
    FileThumbnailStorage(const QString & imageFormat);

    virtual bool store(const QString& key, const QImage& image);
    virtual bool retrieve(const QString& key, QImage* image);
    virtual void remove(const QString& key);
    virtual bool exists(const QString& key);

private:
    QString keyToPath(const QString& key);
    QMutex _cacheLock;
    QSet<QString> _existenceCache;
    QString _imageFormat;
};

// If Filesystem-IO makes trouble, we could consider implementing a
// store in BerkeleyDB or SQLite...

#ifdef TESTING_MEMORY_THUMBNAIL_CACHING
class MemoryThumbnailStorage : public ThumbnailStorage {
public:
    MemoryThumbnailStorage(const char* imageFormat);
    virtual bool store(const QString& key, const QImage& image);
    virtual bool retrieve(const QString& key, QImage* image);
    virtual void remove(const QString&) {}
    virtual bool exists(const QString& key);

private:
    typedef QMap<QString, QByteArray> ImageCache;
    // TODO: add mutex, if tested in multiple threads
    ImageCache _cache;
    const char* const _imageFormat;
};
#endif

}  // end namespace ImageManager

#endif /* THUMBNAIL_STORAGE_H */
