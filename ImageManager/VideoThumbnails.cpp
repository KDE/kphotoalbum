// SPDX-FileCopyrightText: 2012 - 2022 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2024 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "VideoThumbnails.h"

#include <BackgroundJobs/ExtractOneThumbnailJob.h>
#include <BackgroundJobs/HandleVideoThumbnailRequestJob.h>
#include <BackgroundJobs/ReadVideoLengthJob.h>
#include <BackgroundTaskManager/JobManager.h>
#include <MainWindow/FeatureDialog.h>
#include <MainWindow/Window.h>
#include <kpabase/SettingsData.h>
#include <kpathumbnails/VideoThumbnailCache.h>

ImageManager::VideoThumbnails::VideoThumbnails(QObject *parent)
    : QObject(parent)
    , m_pendingRequest(false)
    , m_index(0)
{
    m_cache.resize(10);
    m_activeRequests.reserve(10);
}

void ImageManager::VideoThumbnails::setVideoFile(const DB::FileName &fileName)
{
    m_index = 0;
    m_videoFile = fileName;

    if (MainWindow::Window::theMainWindow()->videoThumbnailCache()->contains(fileName)) {
        return;
    }

    // no video thumbnails without ffmpeg:
    if (!MainWindow::FeatureDialog::hasVideoThumbnailer())
        return;

    if (!fileName.exists())
        return;

    cancelPreviousJobs();
    m_pendingRequest = false;
    for (int i = 0; i < 10; ++i)
        m_cache[i] = QImage();

    BackgroundJobs::ReadVideoLengthJob *lengthJob = new BackgroundJobs::ReadVideoLengthJob(fileName, BackgroundTaskManager::ForegroundCycleRequest);

    for (int i = 0; i < 10; ++i) {
        BackgroundJobs::ExtractOneThumbnailJob *extractJob = new BackgroundJobs::ExtractOneThumbnailJob(fileName, i, BackgroundTaskManager::ForegroundCycleRequest);
        extractJob->addDependency(lengthJob);
        connect(extractJob, &BackgroundJobs::ExtractOneThumbnailJob::completed, this, &VideoThumbnails::gotFrame);
        m_activeRequests.append(QPointer<BackgroundJobs::ExtractOneThumbnailJob>(extractJob));
    }
    BackgroundTaskManager::JobManager::instance()->addJob(lengthJob);
}

void ImageManager::VideoThumbnails::requestNext()
{
    for (int i = 0; i < 10; ++i) {
        m_index = (m_index + 1) % 10;
        const auto &frame = MainWindow::Window::theMainWindow()->videoThumbnailCache()->lookup(m_videoFile, m_index);
        if (!frame.isNull()) {
            Q_EMIT frameLoaded(frame);
            m_pendingRequest = false;
            return;
        }
    }
    m_pendingRequest = true;
}

void ImageManager::VideoThumbnails::gotFrame()
{
    const BackgroundJobs::ExtractOneThumbnailJob *job = qobject_cast<BackgroundJobs::ExtractOneThumbnailJob *>(sender());
    const int index = job->index();
    const DB::FileName thumbnailFile = BackgroundJobs::HandleVideoThumbnailRequestJob::frameName(m_videoFile, index);
    m_cache[index] = QImage(thumbnailFile.absolute());

    if (m_pendingRequest) {
        m_index = index;
        Q_EMIT frameLoaded(m_cache[index]);
    }
}

void ImageManager::VideoThumbnails::cancelPreviousJobs()
{
    for (const QPointer<BackgroundJobs::ExtractOneThumbnailJob> &job : m_activeRequests) {
        if (!job.isNull())
            job->cancel();
    }
    m_activeRequests.clear();
}
// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_VideoThumbnails.cpp"
