// SPDX-FileCopyrightText: 2012-2022 The KPhotoAlbum Development Team
//
// SPDX-License-Identifier: GPL-2.0-or-later

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
    const auto images = DB::ImageDB::instance()->images();
    for (const auto &info : images) {
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
    Q_EMIT completed();
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
