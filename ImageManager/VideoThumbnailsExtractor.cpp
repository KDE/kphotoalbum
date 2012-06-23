/* Copyright 2012  Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "VideoThumbnailsExtractor.h"
#include <QImage>
#include "VideoManager.h"
#include "ExtractOneVideoFrame.h"

ImageManager::VideoThumbnailsExtractor::VideoThumbnailsExtractor( const DB::FileName& fileName, int videoLength, QObject* parent )
    :QObject(parent), m_oneFrameExtractor(new ExtractOneVideoFrame(this)), m_fileName(fileName), m_length(videoLength)
{
    m_frameNumber = -1;
    requestNextFrame();
    connect( m_oneFrameExtractor, SIGNAL(frameFetched(QImage)), this, SLOT(frameFetched(QImage)));
}

void ImageManager::VideoThumbnailsExtractor::requestNextFrame()
{
    m_frameNumber++;
    if ( m_frameNumber == 10 ) {
        emit completed();
        return;
    }

    const double offset = m_length * m_frameNumber / 10;
    m_oneFrameExtractor->extract(m_fileName, offset);
}

void ImageManager::VideoThumbnailsExtractor::frameFetched(const QImage& image)
{
    image.save(frameName(m_fileName,m_frameNumber).absolute(),"JPEG");
    emit frameLoaded(m_frameNumber, image);
    requestNextFrame();
}

DB::FileName ImageManager::VideoThumbnailsExtractor::frameName(const DB::FileName &videoName, int frameNumber)
{
    return DB::FileName::fromRelativePath( ImageManager::VideoManager::pathForRequest(videoName).relative() + QLatin1String("-") + QString::number(frameNumber));
}
