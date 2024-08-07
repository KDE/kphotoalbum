// SPDX-FileCopyrightText: 2012 - 2022 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2024 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IMAGEMANAGER_VIDEOTHUMBNAILS_H
#define IMAGEMANAGER_VIDEOTHUMBNAILS_H

#include <kpabase/FileName.h>

#include <QImage>
#include <QObject>
#include <QPointer>

namespace BackgroundJobs
{
class ExtractOneThumbnailJob;
}

namespace ImageManager
{

class VideoLengthExtractor;
class VideoThumbnailCache;
class VideoThumbnailsExtractor;

/**
  \brief Helper class for extracting videos for thumbnail cycling
  \see \ref videothumbnails
*/
class VideoThumbnails : public QObject
{
    Q_OBJECT
public:
    explicit VideoThumbnails(QObject *parent = nullptr);
    void setVideoFile(const DB::FileName &fileName);

public Q_SLOTS:
    void requestNext();

Q_SIGNALS:
    void frameLoaded(const QImage &);

private Q_SLOTS:
    void gotFrame(const DB::FileName &fileName, int frameNumber, const QImage &frame);

private:
    void cancelPreviousJobs();

    DB::FileName m_videoFile;
    QVector<QImage> m_cache;
    bool m_pendingRequest;
    QVector<QPointer<BackgroundJobs::ExtractOneThumbnailJob>> m_activeRequests;
    int m_index;
    VideoThumbnailCache *m_videoThumbnailCache;
};

}
#endif // IMAGEMANAGER_VIDEOTHUMBNAILS_H
// vi:expandtab:tabstop=4 shiftwidth=4:
