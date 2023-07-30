// SPDX-FileCopyrightText: 2003 David Faure <faure@kde.org>
// SPDX-FileCopyrightText: 2003 Simon Hausmann <hausmann@kde.org>
// SPDX-FileCopyrightText: 2003-2022 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2005-2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2007 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2008 Henner Zeller <h.zeller@acm.org>
// SPDX-FileCopyrightText: 2008-2013 Jan Kundrát <jkt@flaska.net>
// SPDX-FileCopyrightText: 2013 Andreas Neustifter <andreas.neustifter@gmail.com>
// SPDX-FileCopyrightText: 2013-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2018-2019 Robert Krawitz <rlk@alum.mit.edu>

// SPDX-License-Identifier: GPL-2.0-or-later

#include "AsyncLoader.h"

#include "CancelEvent.h"
#include "ImageClientInterface.h"
#include "ImageEvent.h"
#include "ImageLoaderThread.h"
#include "ThumbnailBuilder.h"

#include <BackgroundJobs/HandleVideoThumbnailRequestJob.h>
#include <BackgroundTaskManager/JobManager.h>
#include <MainWindow/FeatureDialog.h>
#include <kpabase/FileExtensions.h>
#include <kpabase/SettingsData.h>
#include <kpathumbnails/ThumbnailCache.h>

#include <QIcon>
#include <QPixmapCache>

ImageManager::AsyncLoader *ImageManager::AsyncLoader::s_instance = nullptr;

// -- Manager --

ImageManager::AsyncLoader *ImageManager::AsyncLoader::instance()
{
    if (!s_instance) {
        s_instance = new AsyncLoader;
        s_instance->init();
    }

    return s_instance;
}

// We need this as a separate method as the s_instance variable will otherwise not be initialized
// corrected before the thread starts.
void ImageManager::AsyncLoader::init()
{

    // Use up to three cores for thumbnail generation. No more than three as that
    // likely will make it less efficient due to three cores hitting the harddisk at the same time.
    // This might limit the throughput on SSD systems, but we likely have a few years before people
    // put all of their pictures on SSDs.
    // rlk 20180515: with improvements to the thumbnail generation code, I've conducted
    // experiments demonstrating benefit even at 2x the number of hyperthreads, even on
    // an HDD.  However, we need to reserve a thread for the UI or it gets very sluggish
    // We need one more core in the computer for the GUI thread, but we won't dedicate it to GUI,
    // as that'd mean that a dual-core box would only have one core decoding images, which would be
    // suboptimal.
    // In case of only one core in the computer, use one core for thumbnail generation
    // jzarl:  It seems that many people have their images on NFS-mounts.
    //         Should we somehow detect this and allocate less threads there?
    //         rlk 20180515: IMO no; if anything, we need more threads to hide the latency of NFS.
    int desiredThreads = Settings::SettingsData::instance()->getThumbnailBuilderThreadCount();
    if (desiredThreads == 0) {
        desiredThreads = qMax(1, qMin(16, QThread::idealThreadCount() - 1));
    }

    for (int i = 0; i < desiredThreads; ++i) {
        ImageLoaderThread *imageLoader = new ImageLoaderThread();
        // The thread is set to the lowest priority to ensure that it doesn't starve the GUI thread.
        m_threadList << imageLoader;
        imageLoader->start(QThread::IdlePriority);
    }
}

bool ImageManager::AsyncLoader::load(ImageRequest *request)
{
    if (m_exitRequested)
        return false;

    // rlk 2018-05-15: Skip this check here.  Even if the check
    // succeeds at this point, it may fail later, and if we're suddenly
    // processing a lot of requests (e. g. a thumbnail build),
    // this may be very I/O-intensive since it actually has to
    // read the inode.
    // silently ignore images not (currently) on disk:
    // if ( ! request->fileSystemFileName().exists() )
    //    return false;

    if (KPABase::isVideo(request->fileSystemFileName())) {
        if (!loadVideo(request))
            return false;
    } else {
        loadImage(request);
    }
    return true;
}

bool ImageManager::AsyncLoader::loadVideo(ImageRequest *request)
{
    if (m_exitRequested)
        return false;

    if (!MainWindow::FeatureDialog::hasVideoThumbnailer())
        return false;

    if (!request->fileSystemFileName().exists())
        return false;

    BackgroundTaskManager::Priority priority = (request->priority() > ThumbnailInvisible)
        ? BackgroundTaskManager::ForegroundThumbnailRequest
        : BackgroundTaskManager::BackgroundVideoThumbnailRequest;

    BackgroundTaskManager::JobManager::instance()->addJob(
        new BackgroundJobs::HandleVideoThumbnailRequestJob(request, priority, MainWindow::Window::theMainWindow()->thumbnailCache()));
    return true;
}

void ImageManager::AsyncLoader::loadImage(ImageRequest *request)
{
    QMutexLocker dummy(&m_lock);
    if (m_exitRequested)
        return;
    auto req = m_currentLoading.constFind(request);
    if (req != m_currentLoading.cend() && m_loadList.isRequestStillValid(request)) {
        // The last part of the test above is needed to not fail on a race condition from AnnotationDialog::ImagePreview, where the preview
        // at startup request the same image numerous time (likely from resize event).
        Q_ASSERT(*req != request);
        delete request;

        return; // We are currently loading it, calm down and wait please ;-)
    }

    // Try harder to find a pending request.  Unfortunately, we can't simply use
    // m_currentLoading.contains() because that will compare pointers
    // when we want to compare values.
    for (req = m_currentLoading.cbegin(); req != m_currentLoading.cend(); req++) {
        ImageRequest *r = *req;
        if (*request == *r) {
            delete request;
            return; // We are currently loading it, calm down and wait please ;-)
        }
    }

    // if request is "fresh" (not yet pending):
    if (m_loadList.addRequest(request))
        m_sleepers.wakeOne();
}

void ImageManager::AsyncLoader::stop(ImageClientInterface *client, StopAction action)
{
    // remove from pending map.
    QMutexLocker requestLocker(&m_lock);
    m_loadList.cancelRequests(client, action);

    // PENDING(blackie) Reintroduce this
    // VideoManager::instance().stop( client, action );
    // Was implemented as m_pending.cancelRequests( client, action );
    // Where m_pending is the RequestQueue
}

int ImageManager::AsyncLoader::activeCount() const
{
    QMutexLocker dummy(&m_lock);
    return m_currentLoading.count();
}

bool ImageManager::AsyncLoader::isExiting() const
{
    return m_exitRequested;
}

ImageManager::ImageRequest *ImageManager::AsyncLoader::next()
{
    QMutexLocker dummy(&m_lock);
    ImageRequest *request = nullptr;
    while (!(request = m_loadList.popNext()))
        m_sleepers.wait(&m_lock);
    m_currentLoading.insert(request);

    return request;
}

void ImageManager::AsyncLoader::requestExit()
{
    m_exitRequested = true;
    ImageManager::ThumbnailBuilder::instance()->cancelRequests();
    m_sleepers.wakeAll();

    // TODO(jzarl): check if we can just connect the finished() signal of the threads to deleteLater()
    //              and exit this function without waiting
    for (QList<ImageLoaderThread *>::iterator it = m_threadList.begin(); it != m_threadList.end(); ++it) {
        while (!(*it)->isFinished()) {
            QThread::msleep(10);
        }
        delete (*it);
    }
}

void ImageManager::AsyncLoader::customEvent(QEvent *ev)
{
    if (ev->type() == ImageEventID) {
        ImageEvent *iev = dynamic_cast<ImageEvent *>(ev);
        if (!iev) {
            Q_ASSERT(iev);
            return;
        }

        ImageRequest *request = iev->loadInfo();

        QMutexLocker requestLocker(&m_lock);
        const bool requestStillNeeded = m_loadList.isRequestStillValid(request);
        m_loadList.removeRequest(request);
        m_currentLoading.remove(request);
        requestLocker.unlock();

        QImage image = iev->image();
        if (!request->loadedOK()) {
            if (m_brokenImage.size() != request->size()) {
                // we can ignore the krazy warning here because we have a valid fallback
                QIcon brokenFileIcon = QIcon::fromTheme(QLatin1String("file-broken")); // krazy:exclude=iconnames
                if (brokenFileIcon.isNull()) {
                    brokenFileIcon = QIcon::fromTheme(QLatin1String("image-x-generic"));
                }
                m_brokenImage = brokenFileIcon.pixmap(request->size()).toImage();
            }

            image = m_brokenImage;
        }

        if (request->isThumbnailRequest())
            MainWindow::Window::theMainWindow()->thumbnailCache()->insert(request->databaseFileName(), image);

        if (requestStillNeeded && request->client()) {
            request->client()->pixmapLoaded(request, image);
        }
        delete request;
    } else if (ev->type() == CANCELEVENTID) {
        CancelEvent *cancelEvent = dynamic_cast<CancelEvent *>(ev);
        Q_ASSERT(cancelEvent);
        cancelEvent->request()->client()->requestCanceled();
    }
}
// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_AsyncLoader.cpp"
