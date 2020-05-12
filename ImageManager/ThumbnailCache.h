/* Copyright (C) 2003-2020 Jesper K. Pedersen <blackie@kde.org>

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
#ifndef THUMBNAILCACHE_H
#define THUMBNAILCACHE_H
#include "CacheFileInfo.h"

#include <DB/FileNameList.h>

#include <QFile>
#include <QHash>
#include <QImage>
#include <QMutex>

template <class Key, class T>
class QCache;

namespace ImageManager
{

class ThumbnailMapping;

/**
 * @brief The ThumbnailCache implements thumbnail storage optimized for speed.
 *
 * ## On-disk storage
 * The problem with the FreeDesktop.org thumbnail storage is that there is one file per image.
 * This means that showing a full page of thumbnails, containing dozens of images requires many
 * file operations.
 *
 * Our storage scheme introduces a single index file (\c thumbnailindex) that contains
 * the index of known thumbnails and their location in the thumbnail storage files (\c thumb-N).
 * The thumbnail storage files contain raw JPEG thumbnail data.
 *
 * This layout creates far less files on the filesystem,
 * the files can be memory-mapped, and because similar sets of images are often
 * shown together, data locality is used to our advantage.
 *
 * ## Caveats
 * Note that thumbnails are only ever added, never deleted from the thumbnail files.
 * Old images remain in the thumbnail files - they are just removed from the index file.
 *
 * ## Further reading
 * - https://specifications.freedesktop.org/thumbnail-spec/thumbnail-spec-latest.html
 */
class ThumbnailCache : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief ThumbnailCache
     * Provide access to a KPhotoAlbum-style thumbnail cache in the given directory.
     * @param baseDirectory the directory in which the \c thumbnailindex file resides.
     */
    ThumbnailCache(const QString &baseDirectory);
    ~ThumbnailCache() override;
    /**
     * @brief Insert a thumbnail for the given file.
     * @param name the image file name
     * @param image the thumbnail data
     */
    void insert(const DB::FileName &name, const QImage &image);
    /**
     * @brief lookup and return the thumbnail for the given file.
     * @param name the image file name
     * @return a QPixmap containing the thumbnail, or a null QPixmap if no thumbnail was found.
     */
    QPixmap lookup(const DB::FileName &name) const;
    /**
     * @brief lookupRawData
     * @param name the image file name
     * @return the raw JPEG thumbnail data or a null QByteArray.
     */
    QByteArray lookupRawData(const DB::FileName &name) const;
    /**
     * @brief Check if the ThumbnailCache contains a thumbnail for the given file.
     * @param name the image file name
     * @return \c true if the thumbnail exists, \c false otherwise.
     */
    bool contains(const DB::FileName &name) const;
    /**
     * @brief "Forget" the thumbnail for an image.
     * @param name the image file name
     */
    void removeThumbnail(const DB::FileName &name);
    /**
     * @brief "Forget" the thumbnails for the given images.
     * Like removeThumbnail(), but for a list of images
     * @param names a list of image file names
     */
    void removeThumbnails(const DB::FileNameList &names);

public slots:
    /**
     * @brief Save the thumbnail cache to disk.
     */
    void save() const;
    /**
     * @brief Invalidate the ThumbnailCache and remove the thumbnail files and index.
     */
    void flush();

signals:
    /**
     * @brief doSave is emitted when save() is called.
     * This signal is more or less an internal signal.
     */
    void doSave() const;

private:
    /**
     * @brief load the \c thumbnailindex file if possible.
     * This function populates the thumbnail hash, but does not
     * load any actual thumbnail data.
     * If the file does not exist, or if it is not compatible,
     * then it is discarded.
     */
    void load();
    QString fileNameForIndex(int index) const;
    QString thumbnailPath(const QString &fileName) const;

    const QString m_baseDir;
    QHash<DB::FileName, CacheFileInfo> m_hash;
    mutable QHash<DB::FileName, CacheFileInfo> m_unsavedHash;
    /* Protects accesses to the data (hash and unsaved hash) */
    mutable QMutex m_dataLock;
    /* Prevents multiple saves from happening simultaneously */
    mutable QMutex m_saveLock;
    /* Protects writing thumbnails to disk */
    mutable QMutex m_thumbnailWriterLock;
    int m_currentFile;
    int m_currentOffset;
    mutable QTimer *m_timer;
    mutable bool m_needsFullSave;
    mutable bool m_isDirty;
    void saveFull() const;
    void saveIncremental() const;
    void saveInternal() const;
    void saveImpl() const;

    /**
     * Holds an in-memory cache of thumbnail files.
     */
    mutable QCache<int, ThumbnailMapping> *m_memcache;
    mutable QFile *m_currentWriter;
};

/**
 * @brief defaultThumbnailDirectory
 * @return the default thumbnail (sub-)directory name, e.g. ".thumbnails"
 */
QString defaultThumbnailDirectory();
}

#endif /* THUMBNAILCACHE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
