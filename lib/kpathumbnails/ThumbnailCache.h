// SPDX-FileCopyrightText: 2003-2020 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef KPATHUMBNAILS_THUMBNAILCACHE_H
#define KPATHUMBNAILS_THUMBNAILCACHE_H
#include "CacheFileInfo.h"

#include <kpabase/FileNameList.h>

#include <QDir>
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
     * @brief insert inserts raw JPEG-encoded data into the thumbnail database.
     * It is recommended that you use insert(const DB::FileName&, const QImage&) if possible.
     * @param name the image file name
     * @param thumbnailData the JPEG encoded image data
     */
    void insert(const DB::FileName &name, const QByteArray &thumbnailData);
    /**
     * @brief lookup and return the thumbnail for the given file.
     * Note: this method requires a GuiApplication to exist.
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

    /**
     * @brief thumbnailSize
     * Usually, this is the size of the thumbnails in the cache.
     * If the index file was converted from an older file version (4),
     * the size is read from the configuration file.
     * @return the current thumbnail size.
     */
    int thumbnailSize() const;

    /**
     * @brief Returns the file format version of the thumbnailindex file currently on disk.
     *
     * Usually, this is equal to the current version, but if an old ThumbnailCache
     * that is still compatible with this version of KPhotoAlbum is loaded and was not yet stored,
     * it may differ.
     * @return 4 or 5 if the cache has been written to disk, or -1 for a fresh, unsaved cache.
     */
    int actualFileVersion() const;

    /**
     * @brief Version of the tumbnailindex file when saved.
     * @return The file format version of the thumbnailindex file.
     */
    static int preferredFileVersion();

    /**
     * @brief Check all thumbnails for consistency with thumbnailSize().
     * Only the thumbnails which are saved to disk are checked.
     * If you have changed changed the cache you need to save it to guarantee correct results.
     * @return all thumbnails that do not match the expected image dimensions.
     */
    DB::FileNameList findIncorrectlySizedThumbnails() const;

    /**
     * @brief size
     * @return the number of (saved) thumbnails in the cache
     */
    int size() const;

    /**
     * @brief Compacts the on-disk storage for the cache by discarding stale data from its files.
     * To ensure consistency, this method also saves the cache.
     */
    void vacuum();

public slots:
    /**
     * @brief Save the thumbnail cache to disk.
     * Note: this method emits an internal signal which calls the actual save implementation.
     * Therefore, saving may not be finished when this method returns.
     */
    void save() const;
    /**
     * @brief Invalidate the ThumbnailCache and remove the thumbnail files and index.
     */
    void flush();

    /**
     * @brief setThumbnailSize sets the thumbnail size recorded in the thumbnail index file.
     * If the value changes, the thumbnail cache is invalidated.
     * Except minimal sanity checks, no bounds for thumbnailSize are enforced.
     * @param thumbnailSize
     */
    void setThumbnailSize(int thumbnailSize);
signals:
    /**
     * @brief doSave is emitted when save() is called.
     * This signal is more or less an internal signal.
     */
    void doSave() const;

    /**
     * @brief cacheInvalidated is emitted when the thumbnails are no longer valid.
     * This usually happens when the thumbnail size changed.
     * This signal is *not* emitted when the cache was flushed by explicit request.
     * @see cacheFlushed()
     */
    void cacheInvalidated();

    /**
     * @brief cacheFlushed is emitted if the cache was flushed by explicit request.
     * @see flush()
     * @see cacheInvalidated()
     */
    void cacheFlushed();

    /**
     * @brief saveComplete is emitted after the thumbnail cache was successfully saved.
     * At the time the signal is emitted, the cache is not dirty (i.e. a full save was performed).
     */
    void saveComplete() const;

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
    /**
     * @brief thumbnailPath
     * @param utf8FileName the name of the file (does not have to exist), UTF-8 encoded
     * @return the file path for the named file in the thumbnail directory
     */
    QString thumbnailPath(const char *utf8FileName) const;
    /**
     * @brief thumbnailPath
     * @param fileName the name of the file (does not have to exist)
     * @return the file path for the named file in the thumbnail directory
     */
    QString thumbnailPath(const QString &fileName) const;

    // mutable because saveIncremental is const
    mutable int m_fileVersion = -1;
    int m_thumbnailSize = -1;
    const QDir m_baseDir;
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
    /**
     * @brief saveFull stores the full contents of the index file.
     * I.e. the whole file is rewritten.
     */
    void saveFull() const;
    /**
     * @brief saveIncremental appends all unsaved hash entries to the index file.
     */
    void saveIncremental() const;
    /**
     * @brief saveInternal checks whether a full save is requested or needed and calls the incremental or full save accordingly.
     */
    void saveInternal() const;
    /**
     * @brief saveImpl calls saveInternal and resets the save timer.
     */
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

#endif /* KPATHUMBNAILS_THUMBNAILCACHE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
