// SPDX-FileCopyrightText: 2024 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VIDEOTHUMBNAILCACHE_H
#define VIDEOTHUMBNAILCACHE_H

#include <QCache>
#include <QDir>
#include <QImage>
#include <QObject>
#include <QVector>

#include <kpabase/FileName.h>
#include <kpabase/FileNameList.h>

namespace ImageManager
{

/**
 * @brief The VideoThumbnailCache class complements the ThumbnailCache class by adding support for animated video thumbnails.
 *
 * ## Comparison and differences to the ThumbnailCache
 *
 * - ThumbnailCache stores thumbnails for both video and image files.
 * - VideoThumbnailCache only stores animated thumbnails for video files.
 * - ThumbnailCache is especially tuned for performance, so that there is minimal delay when browsing the thumbnail view.
 * - VideoThumbnailCache is optimized for animating a single video at a time.
 *
 * @see ThumbnailCache
 */
class VideoThumbnailCache : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief VideoThumbnailCache
     * Provide access to a KPhotoAlbum-style video thumbnail storage, i.e. to the cycling thumbnail images usually visible when a video is hovered on.
     * @param baseDirectory The (relative) directory for the video thumbnail directory (@see defaultVideoThumbnailDirectory).
     * @param parent
     */
    explicit VideoThumbnailCache(const QString &baseDirectory, QObject *parent = nullptr);

    /**
     * @brief lookup and return the thumbnail frames for the given file.
     * @param name the video file name
     * @return a QVector containing all frames, or an empty QVector.
     */
    QVector<QImage> lookup(const DB::FileName &name) const;

    /**
     * @brief contains
     * @param name
     * @return \c true, if all frames for the video file are found, \c false otherwise.
     */
    bool contains(const DB::FileName &name) const;

    void removeThumbnail(const DB::FileName &name);
    void removeThumbnails(const DB::FileNameList &names);

private:
    const QDir m_baseDir;
    QCache<QString, QVector<QImage>> m_memcache;

    /**
     * @brief nameHash transforms the videoName into a name used as base for the video thumbnail frame files.
     * E.g. a the file name 'example.jpg' might be transformed into something like '6f291fb338dc2c448811cc1f4aea3806'
     * @param videoName the name of the video file
     * @return a string suitable as file name prefix of the video frame files
     * @see frameName
     */
    QString nameHash(const DB::FileName &videoName) const;

    /**
     * @brief frameName
     * @param videoName
     * @param frameNumber a frame number between 0 and 9.
     * @return the filename for the given frame of the video thumbnail for \c videoName
     */
    DB::FileName frameName(const DB::FileName &videoName, int frameNumber) const;
};

/**
 * @brief defaultVideoThumbnailDirectory
 * @return the default video thumbnail (sub-)directory name, e.g. ".thumbnails"
 */
QString defaultVideoThumbnailDirectory();
} // namespace ImageManager

#endif // VIDEOTHUMBNAILCACHE_H
