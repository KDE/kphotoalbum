// SPDX-FileCopyrightText: 2012-2022 Jesper K. Pedersen <blackie@kde.org>
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

class VideoThumbnailsExtractor;
class VideoLengthExtractor;

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
    void gotFrame();

private:
    bool loadFramesFromCache(const DB::FileName &fileName);
    void cancelPreviousJobs();

    DB::FileName m_videoFile;
    QVector<QImage> m_cache;
    bool m_pendingRequest;
    QVector<QPointer<BackgroundJobs::ExtractOneThumbnailJob>> m_activeRequests;
    int m_index;
};

}
#endif // IMAGEMANAGER_VIDEOTHUMBNAILS_H
// vi:expandtab:tabstop=4 shiftwidth=4:
