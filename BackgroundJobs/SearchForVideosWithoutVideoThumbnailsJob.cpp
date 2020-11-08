/* SPDX-FileCopyrightText: 2012-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SearchForVideosWithoutVideoThumbnailsJob.h"

#include "ExtractOneThumbnailJob.h"
#include "HandleVideoThumbnailRequestJob.h"
#include "ReadVideoLengthJob.h"

#include <BackgroundTaskManager/JobInfo.h>
#include <BackgroundTaskManager/JobManager.h>
#include <DB/ImageDB.h>
#include <DB/ImageInfo.h>

#include <KLocalizedString>
#include <QFile>

using namespace BackgroundJobs;

void BackgroundJobs::SearchForVideosWithoutVideoThumbnailsJob::execute()
{
    const auto images = DB::ImageDB::instance()->images();

    for (const auto &info : images) {
        if (!info->isVideo())
            continue;

        // silently ignore videos not (currently) on disk:
        if (!info->fileName().exists())
            continue;

        const DB::FileName thumbnailName = BackgroundJobs::HandleVideoThumbnailRequestJob::frameName(info->fileName(), 9);
        if (thumbnailName.exists())
            continue;

        BackgroundJobs::ReadVideoLengthJob *readVideoLengthJob = new BackgroundJobs::ReadVideoLengthJob(info->fileName(), BackgroundTaskManager::BackgroundVideoPreviewRequest);

        for (int i = 0; i < 10; ++i) {
            ExtractOneThumbnailJob *extractJob = new ExtractOneThumbnailJob(info->fileName(), i, BackgroundTaskManager::BackgroundVideoPreviewRequest);
            extractJob->addDependency(readVideoLengthJob);
        }

        BackgroundTaskManager::JobManager::instance()->addJob(readVideoLengthJob);
    }
    emit completed();
}

QString BackgroundJobs::SearchForVideosWithoutVideoThumbnailsJob::title() const
{
    return i18n("Searching for videos without video thumbnails");
}

QString BackgroundJobs::SearchForVideosWithoutVideoThumbnailsJob::details() const
{
    return QString();
}

SearchForVideosWithoutVideoThumbnailsJob::SearchForVideosWithoutVideoThumbnailsJob()
    : JobInterface(BackgroundTaskManager::BackgroundVideoPreviewRequest)
{
}

// vi:expandtab:tabstop=4 shiftwidth=4:
