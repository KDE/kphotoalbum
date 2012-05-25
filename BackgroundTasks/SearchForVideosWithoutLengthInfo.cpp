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

#include "SearchForVideosWithoutLengthInfo.h"
#include <DB/ImageDB.h>
#include <DB/ImageInfo.h>
#include <DB/IdList.h>
#include "JobManager.h"
#include "ReadVideoLengthJob.h"

/**
  \class BackgroundTasks::SearchForVideosWithoutLengthInfo
  \brief Task for searching the database for videos without length information
*/

void BackgroundTasks::SearchForVideosWithoutLengthInfo::execute()
{
    const DB::FileNameList images = DB::ImageDB::instance()->images();
    Q_FOREACH( const DB::FileName& image, images ) {
        const DB::ImageInfoPtr info = image.info();
        if ( !info->isVideo() )
            continue;
        int length = info->videoLength();
        if ( length == -1 ) {
            JobManager::instance()->addJob( new ReadVideoLengthJob(info->fileName()) );
        }
    }
    emit completed();
}
