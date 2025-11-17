// SPDX-FileCopyrightText: 2012-2022 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2013-2024 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2022 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "SearchForVideosWithoutVideoThumbnailsJob.h"

#include "ExtractOneThumbnailJob.h"
#include "HandleVideoThumbnailRequestJob.h"
#include "ReadVideoMetaDataJob.h"
#include "kpathumbnails/VideoThumbnailCache.h"

#include <BackgroundTaskManager/JobInfo.h>
#include <BackgroundTaskManager/JobManager.h>
#include <DB/ImageDB.h>
#include <DB/ImageInfo.h>
#include <MainWindow/Window.h>
#include <KLocalizedString>
#include <QFile>

using namespace BackgroundJobs;

void BackgroundJobs::SearchForVideosWithoutVideoThumbnailsJob::execute()
{
    const auto images = DB::ImageDB::instance()->images();
    const auto *videoThumbnailCache = MainWindow::Window::theMainWindow()->videoThumbnailCache();

    for (const auto &info : images) {
        if (!info->isVideo())
            continue;

        // silently ignore videos not (currently) on disk:
        if (!info->fileName().exists())
            continue;

        if (videoThumbnailCache->contains(info->fileName()))
            continue;

        BackgroundJobs::ReadVideoMetaDataJob *readVideoLengthJob = new BackgroundJobs::ReadVideoMetaDataJob(info->fileName(), BackgroundTaskManager::BackgroundVideoPreviewRequest);

        for (int i = 0; i < 10; ++i) {
            if (videoThumbnailCache->contains(info->fileName(), i))
                continue;
            ExtractOneThumbnailJob *extractJob = new ExtractOneThumbnailJob(info->fileName(), i, BackgroundTaskManager::BackgroundVideoPreviewRequest);
            extractJob->addDependency(readVideoLengthJob);
            connect(extractJob, &ExtractOneThumbnailJob::frameAvailable, videoThumbnailCache, &ImageManager::VideoThumbnailCache::insertThumbnail);
        }

        BackgroundTaskManager::JobManager::instance()->addJob(readVideoLengthJob);
    }
    Q_EMIT completed();
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

#include "moc_SearchForVideosWithoutVideoThumbnailsJob.cpp"
