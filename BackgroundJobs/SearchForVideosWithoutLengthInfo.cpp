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

#include "ReadVideoLengthJob.h"

#include <BackgroundTaskManager/JobInfo.h>
#include <BackgroundTaskManager/JobManager.h>
#include <DB/ImageDB.h>
#include <DB/ImageInfo.h>

#include <KLocalizedString>

/**
  \class BackgroundJobs::SearchForVideosWithoutLengthInfo
  \brief Task for searching the database for videos without length information
*/

BackgroundJobs::SearchForVideosWithoutLengthInfo::SearchForVideosWithoutLengthInfo()
    : BackgroundTaskManager::JobInterface(BackgroundTaskManager::BackgroundVideoInfoRequest)
{
}

void BackgroundJobs::SearchForVideosWithoutLengthInfo::execute()
{
    const DB::FileNameList images = DB::ImageDB::instance()->files();
    for (const DB::FileName &image : images) {
        const DB::ImageInfoPtr info = image.info();
        if (!info->isVideo())
            continue;
        // silently ignore videos not (currently) on disk:
        if (!info->fileName().exists())
            continue;
        int length = info->videoLength();
        if (length == -1) {
            BackgroundTaskManager::JobManager::instance()->addJob(
                new BackgroundJobs::ReadVideoLengthJob(info->fileName(), BackgroundTaskManager::BackgroundVideoPreviewRequest));
        }
    }
    emit completed();
}

QString BackgroundJobs::SearchForVideosWithoutLengthInfo::title() const
{
    return i18n("Search for videos without length information");
}

QString BackgroundJobs::SearchForVideosWithoutLengthInfo::details() const
{
    return QString();
}
// vi:expandtab:tabstop=4 shiftwidth=4:
