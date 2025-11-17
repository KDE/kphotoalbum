// SPDX-FileCopyrightText: 2012 - 2022 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2024 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "VideoThumbnails.h"

#include <BackgroundJobs/ExtractOneThumbnailJob.h>
#include <BackgroundJobs/HandleVideoThumbnailRequestJob.h>
#include <BackgroundJobs/ReadVideoMetaDataJob.h>
#include <BackgroundTaskManager/JobManager.h>
#include <MainWindow/FeatureDialog.h>
#include <MainWindow/Window.h>
#include <kpabase/Logging.h>
#include <kpabase/SettingsData.h>
#include <kpathumbnails/VideoThumbnailCache.h>

ImageManager::VideoThumbnails::VideoThumbnails(QObject *parent)
    : QObject(parent)
    , m_pendingRequest(false)
    , m_index(0)
    , m_videoThumbnailCache(MainWindow::Window::theMainWindow()->videoThumbnailCache())
{
    m_cache.resize(10);
    m_activeRequests.reserve(10);
}

void ImageManager::VideoThumbnails::setVideoFile(const DB::FileName &fileName)
{
    m_index = 0;
    m_videoFile = fileName;

    if (m_videoThumbnailCache->contains(fileName)) {
        auto lookupResult = m_videoThumbnailCache->lookup(fileName);
        // if a thumbnail frame can not be read, the result is empty and cannot be used:
        if (!lookupResult.isEmpty()) {
            m_cache = lookupResult;
            return;
        }
    }

    for (int i = 0; i < 10; ++i)
        m_cache[i] = QImage();

    // no video thumbnails without ffmpeg:
    if (!MainWindow::FeatureDialog::hasVideoThumbnailer())
        return;

    if (!fileName.exists())
        return;

    cancelPreviousJobs();
    m_pendingRequest = false;

    BackgroundJobs::ReadVideoMetaDataJob *lengthJob = new BackgroundJobs::ReadVideoMetaDataJob(fileName, BackgroundTaskManager::ForegroundCycleRequest);

    qCDebug(ImageManagerLog) << "VideoThumbnails: Creating thumbnails for" << fileName.relative();
    for (int i = 0; i < 10; ++i) {
        BackgroundJobs::ExtractOneThumbnailJob *extractJob = new BackgroundJobs::ExtractOneThumbnailJob(fileName, i, BackgroundTaskManager::ForegroundCycleRequest);
        extractJob->addDependency(lengthJob);
        connect(extractJob, &BackgroundJobs::ExtractOneThumbnailJob::frameAvailable, m_videoThumbnailCache, &VideoThumbnailCache::insertThumbnail);
        // don't go through videoThumbnailCache if we also get the frame directly from the signal:
        connect(extractJob, &BackgroundJobs::ExtractOneThumbnailJob::frameAvailable, this, &VideoThumbnails::gotFrame);
        m_activeRequests.append(QPointer<BackgroundJobs::ExtractOneThumbnailJob>(extractJob));
    }
    BackgroundTaskManager::JobManager::instance()->addJob(lengthJob);
}

void ImageManager::VideoThumbnails::requestNext()
{
    for (int i = 0; i < 10; ++i) {
        m_index = (m_index + 1) % 10;
        // check local cache first
        auto &frame = m_cache[m_index];
        if (frame.isNull())
            frame = m_videoThumbnailCache->lookup(m_videoFile, m_index);
        if (!frame.isNull()) {
            Q_EMIT frameLoaded(frame);
            m_pendingRequest = false;
            return;
        }
    }
    m_pendingRequest = true;
}

void ImageManager::VideoThumbnails::gotFrame(const DB::FileName &fileName, int frameNumber, const QImage &frame)
{
    if (fileName != m_videoFile)
        return;

    m_cache[frameNumber] = frame;

    if (m_pendingRequest) {
        m_index = frameNumber;
        Q_EMIT frameLoaded(m_cache[frameNumber]);
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
