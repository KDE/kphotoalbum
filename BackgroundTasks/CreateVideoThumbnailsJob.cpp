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

#include "CreateVideoThumbnailsJob.h"
#include <ImageManager/VideoThumbnailsExtractor.h>
#include <DB/ImageDB.h>
#include <DB/ImageInfo.h>

BackgroundTasks::CreateVideoThumbnailsJob::CreateVideoThumbnailsJob(const DB::FileName &fileName)
    :m_fileName(fileName)
{
}

void BackgroundTasks::CreateVideoThumbnailsJob::execute()
{
    const DB::ImageInfoPtr info = DB::ImageDB::instance()->info(m_fileName);
    const int length = info->videoLength();
    if ( length <= 0 ) {
        emit completed();
        return;
    }

    ImageManager::VideoThumbnailsExtractor* extractor = new ImageManager::VideoThumbnailsExtractor( m_fileName, length );
    connect(extractor, SIGNAL(completed()), this, SIGNAL(completed()));
}

