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

#include "VideoThumbnails.h"
#include "VideoThumbnailsExtractor.h"
#include "VideoLengthExtractor.h"
#include <QFile>

ImageManager::VideoThumbnails::VideoThumbnails(QObject *parent) :
    QObject(parent), m_extractor(0)
{
    m_cache.resize(10);
    m_lengthExtractor = new VideoLengthExtractor(this);
    connect( m_lengthExtractor, SIGNAL(lengthFound(int)), this, SLOT(setLength(int)));
}

void ImageManager::VideoThumbnails::setVideoFile(const DB::FileName &fileName)
{
    if ( loadFramesFromCache(fileName.absolute()) ) // ZZZ
        return;

    delete m_extractor;
    m_extractor = 0;

    m_videoFile = fileName;
    m_pendingRequest = 0;
    for ( int i= 0; i < 10; ++i )
        m_cache[i] = QImage();

    m_lengthExtractor->extract(fileName.absolute()); // ZZZ
}

void ImageManager::VideoThumbnails::requestFrame(int fraction)
{
    if ( m_cache[fraction].isNull() )
        m_pendingRequest = fraction;
    else
        emit frameLoaded(m_cache[fraction]);
}

void ImageManager::VideoThumbnails::gotFrame(int index, const QImage &image)
{
    m_cache[index]=image;

    if ( m_pendingRequest == index )
        emit frameLoaded(image);
}

void ImageManager::VideoThumbnails::setLength(int length)
{
    m_extractor = new ImageManager::VideoThumbnailsExtractor(m_videoFile, length);
    connect( m_extractor, SIGNAL(frameLoaded(int,QImage)), this, SLOT(gotFrame(int,QImage)));
}

bool ImageManager::VideoThumbnails::loadFramesFromCache(const QString& fileName)
{
    for (int i=0; i <10; ++i) {
        const QString thumbnailFile = VideoThumbnailsExtractor::frameName(DB::FileName::fromAbsolutePath(fileName), i); // ZZZ
        if ( !QFile::exists(thumbnailFile))
            return false;

        QImage image(thumbnailFile);
        if ( image.isNull() )
            return false;

        m_cache[i] = image;
    }
    return true;
}
