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

#ifndef VIDEOTHUMBNAILEXTRACTOR_H
#define VIDEOTHUMBNAILEXTRACTOR_H

#include <QObject>
#include <QImage>
#include <DB/FileName.h>

namespace ImageManager {

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
    explicit VideoThumbnails(QObject *parent = 0);
    void setVideoFile( const DB::FileName& fileName );
    void requestFrame(int fraction); // 0..9

signals:
    void frameLoaded( const QImage& );

private slots:
    void gotFrame(int index, const QImage& image );
    void setLength(int length);

private:
    bool loadFramesFromCache(const DB::FileName& fileName);

    DB::FileName m_videoFile;
    QVector<QImage> m_cache;
    ImageManager::VideoLengthExtractor* m_lengthExtractor;
    ImageManager::VideoThumbnailsExtractor* m_extractor;
    int m_pendingRequest;
};

}
#endif // VIDEOTHUMBNAILEXTRACTOR_H
