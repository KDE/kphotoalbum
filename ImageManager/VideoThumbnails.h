/* Copyright (C) 2012 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef IMAGEMANAGER_VIDEOTHUMBNAILS_H
#define IMAGEMANAGER_VIDEOTHUMBNAILS_H

#include <DB/FileName.h>
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

public slots:
    void requestNext();

signals:
    void frameLoaded(const QImage &);

private slots:
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
