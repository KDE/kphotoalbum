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
 * - VideoThumbnailCache frames are not scaled to the current thumbnail size.
 *
 * @see ThumbnailCache
 */
class VideoThumbnailCache : public QObject
{
    Q_OBJECT
public:
    enum class LookupMode {
        Complete, ///< Require all frames
        Partial ///< Allow incomplete video thumbnails.
    };
    /**
     * @brief VideoThumbnailCache
     * Provide access to a KPhotoAlbum-style video thumbnail storage, i.e. to the cycling thumbnail images usually visible when a video is hovered on.
     * @param baseDirectory The directory for the video thumbnail directory (@see defaultVideoThumbnailDirectory). Must be a subdirectory of the image directory.
     * @param parent
     */
    explicit VideoThumbnailCache(const QString &baseDirectory, QObject *parent = nullptr);

    /**
     * @brief lookup and return the thumbnail frames for the given file.
     * @param name the video file name
     * @param mode Use \c LookupMode::Complete to reject video thumnails with missing frames, \c LookupMode::Partial to use null QImages for missing frames.
     * @return a QVector containing all frames, or an empty QVector.
     */
    const QVector<QImage> lookup(const DB::FileName &name, LookupMode mode = LookupMode::Complete) const;

    /**
     * @brief lookup and return a specific thumbnail frameNumber for the given file.
     * @param name the video file name
     * @param frameNumber between 0 and \c numberOfFrames()
     * @return a QImage for the requested frameNumber, or a null QImage if the frameNumber was found
     */
    QImage lookup(const DB::FileName &name, int frameNumber) const;

    /**
     * @brief lookupStillFrame looks up the thumbnail frame that is used for the static thumbnail.
     * I.e. this is the frame that is also stored in the ThumbnailCache. In contrast to the version in the ThumbnailCache, the video thumbnail is not scaled.
     * @param name
     * @return the still frame, or a default-constructed QImage
     */
    QImage lookupStillFrame(const DB::FileName &name) const;

    /**
     * @brief contains
     * @param name the video file name
     * @return \c true, if all frames for the video file are found, \c false otherwise.
     */
    bool contains(const DB::FileName &name) const;

    /**
     * @brief contains
     * @param name the video file name
     * @param frameNumber between 0 and \c numberOfFrames()
     * @return \c true, if the given frame for the video file is found, \c false otherwise.
     */
    bool contains(const DB::FileName &name, int frameNumber) const;

    /**
     * @brief insertThumbnail inserts a single thumbnail frame for the given image.
     *
     * The frame is stored to disk. Once all frames have been stored to disk, \c contains is \c true and \c lookup can be used to retrieve some or all frames.
     * I.e. even if \c lookup(const DB::FileName&, int) is called for a frame that has already been inserted, the frame is only available once all frames have been stored.
     *
     * If the \c frameNumber is 0, the frame is stored as the new still frame (@see lookupStillFrame).
     *
     * @param name the video file name
     * @param frameNumber the frame number between 0 and \c numberOfFrames()
     * @param image the thumbnail frame
     */
    void insertThumbnail(const DB::FileName &name, int frameNumber, const QImage &image);

    /**
     * @brief blockThumbnail marks a frame as (permanently) unavailable.
     * This method can be used if an extracted video thumbnail frame turns out to be invalid.
     * That way, the other frames can be used even if a single frame extraction fails.
     * @param name
     * @param frameNumber between 0 and \c numberOfFrames()
     */
    void blockThumbnail(const DB::FileName &name, int frameNumber);

    /**
     * @brief removeThumbnail removes all frames for the given file name.
     *
     * @param name the video file name
     */
    void removeThumbnail(const DB::FileName &name);
    /**
     * @brief removeThumbnails removes all frames for the given file names.
     * @param names a list of video file names
     */
    void removeThumbnails(const DB::FileNameList &names);

    /**
     * @brief numberOfFrames
     * @return the number of frames of a video thumbnail.
     */
    int numberOfFrames() const;

    /**
     * @brief stillFrameIndex compares the still frame file to the other thumbnail frames and returns the matching frame.
     * @param name
     * @return The frame number greater or equal to  0 and smaller than \c numberOfFrames() on success, or \c numberOfFrames() if no match is found.
     */
    int stillFrameIndex(const DB::FileName &name) const;

    /**
     * @brief setStillFrame sets the still frame for the given video file to the given frame number.
     * If the frame thumbnail is not found, then the still frame is not changed.
     * @param name
     * @param frameNumber between 0 and \c numberOfFrames()
     * @return \c true, if the thumbnail was set, \c false otherwise.
     */
    bool setStillFrame(const DB::FileName &name, int frameNumber);

Q_SIGNALS:
    void frameUpdated(const DB::FileName &name, int frameNumber);

private:
    const QDir m_baseDir;
    mutable QCache<QString, QVector<QImage>> m_memcache;

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
     * @param frameNumber between 0 and \c numberOfFrames()
     * @return the filename for the given frame of the video thumbnail for \c videoName
     */
    DB::FileName frameName(const DB::FileName &videoName, int frameNumber) const;

    /**
     * @brief stillFrameName
     * @param videoName
     * @return the filename for the still frame of the video thumbnail for \c videoName
     */
    DB::FileName stillFrameName(const DB::FileName &videoName) const;
};

/**
 * @brief defaultVideoThumbnailDirectory
 * @return the default video thumbnail (sub-)directory name, e.g. ".thumbnails"
 */
QString defaultVideoThumbnailDirectory();
} // namespace ImageManager

#endif // VIDEOTHUMBNAILCACHE_H
