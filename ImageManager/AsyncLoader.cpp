/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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

#include "AsyncLoader.h"

#include <QIcon>
#include <QPixmapCache>

#include <BackgroundJobs/HandleVideoThumbnailRequestJob.h>
#include <BackgroundTaskManager/JobManager.h>
#include <ImageManager/ImageClientInterface.h>
#include <MainWindow/FeatureDialog.h>
#include <Utilities/Util.h>

#include "CancelEvent.h"
#include "ImageEvent.h"
#include "ImageLoaderThread.h"
#include "ThumbnailCache.h"
#include "ThumbnailBuilder.h"

ImageManager::AsyncLoader* ImageManager::AsyncLoader::s_instance = nullptr;

// -- Manager --

ImageManager::AsyncLoader* ImageManager::AsyncLoader::instance()
{
    if ( !s_instance )  {
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
    // TODO(isilmendil): It seems that many people have their images on NFS-mounts.
    //                   Should we somehow detect this and allocate less threads there?
    //                   rlk 20180515: IMO no; if anything, we need more threads to hide
    //                   the latency of NFS.
    const int cores = qMax( 1, qMin( 16, QThread::idealThreadCount() - 1 ) );
    m_exitRequested = false;

    for ( int i = 0; i < cores; ++i) {
        ImageLoaderThread* imageLoader = new ImageLoaderThread();
        // The thread is set to the lowest priority to ensure that it doesn't starve the GUI thread.
        m_threadList << imageLoader;
        imageLoader->start( QThread::IdlePriority );
    }
}

bool ImageManager::AsyncLoader::load( ImageRequest* request )
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

    if ( Utilities::isVideo( request->fileSystemFileName() ) ) {
        if (!loadVideo( request ))
            return false;
    } else {
        loadImage( request );
    }
    return true;
}

bool ImageManager::AsyncLoader::loadVideo( ImageRequest* request)
{
    if (m_exitRequested)
        return false;

    if ( ! MainWindow::FeatureDialog::hasVideoThumbnailer() )
        return false;

    BackgroundTaskManager::Priority priority =
            (request->priority() > ThumbnailInvisible)
            ?  BackgroundTaskManager::ForegroundThumbnailRequest
             : BackgroundTaskManager::BackgroundVideoThumbnailRequest;

    BackgroundTaskManager::JobManager::instance()->addJob(
                new BackgroundJobs::HandleVideoThumbnailRequestJob(request,priority));
    return true;
}

void ImageManager::AsyncLoader::loadImage( ImageRequest* request )
{
    QMutexLocker dummy( &m_lock );
    if (m_exitRequested)
        return;
    QSet<ImageRequest*>::const_iterator req = m_currentLoading.find( request );
    if ( req != m_currentLoading.end() && m_loadList.isRequestStillValid( request ) ) {
        // The last part of the test above is needed to not fail on a race condition from AnnotationDialog::ImagePreview, where the preview
        // at startup request the same image numerous time (likely from resize event).
        Q_ASSERT ( *req != request);
        delete request;

        return; // We are currently loading it, calm down and wait please ;-)
    }

    // Try harder to find a pending request.  Unfortunately, we can't simply use
    // m_currentLoading.contains() because that will compare pointers
    // when we want to compare values.
    for (req = m_currentLoading.begin(); req != m_currentLoading.end(); req++) {
        ImageRequest *r = *req;
        if (*request == *r) {
            delete request;
            return; // We are currently loading it, calm down and wait please ;-)
        }
    }
            
    // if request is "fresh" (not yet pending):
    if (m_loadList.addRequest( request ))
        m_sleepers.wakeOne();
}

void ImageManager::AsyncLoader::stop( ImageClientInterface* client, StopAction action )
{
    // remove from pending map.
    QMutexLocker requestLocker( &m_lock );
    m_loadList.cancelRequests( client, action );

    // PENDING(blackie) Reintroduce this
    // VideoManager::instance().stop( client, action );
    // Was implemented as m_pending.cancelRequests( client, action );
    // Where m_pending is the RequestQueue
}

int ImageManager::AsyncLoader::activeCount() const
{
    QMutexLocker dummy( &m_lock );
    return m_currentLoading.count();
}

bool ImageManager::AsyncLoader::isExiting() const
{
    return m_exitRequested;
}

ImageManager::ImageRequest* ImageManager::AsyncLoader::next()
{
    QMutexLocker dummy( &m_lock );
    ImageRequest* request = nullptr;
    while ( !( request = m_loadList.popNext() ) )
        m_sleepers.wait( &m_lock );
    m_currentLoading.insert( request );

    return request;
}

void ImageManager::AsyncLoader::requestExit()
{
    m_exitRequested = true;
    ImageManager::ThumbnailBuilder::instance()->cancelRequests();
    m_sleepers.wakeAll();

    // TODO(jzarl): check if we can just connect the finished() signal of the threads to deleteLater()
    //              and exit this function without waiting
    for (QList<ImageLoaderThread*>::iterator it = m_threadList.begin(); it != m_threadList.end(); ++it ) {
        while (! (*it)->isFinished()) {
            QThread::msleep(10);
        }
        delete (*it);
    }
}

void ImageManager::AsyncLoader::customEvent( QEvent* ev )
{
    if ( ev->type() == ImageEventID )  {
        ImageEvent* iev = dynamic_cast<ImageEvent*>( ev );
        if ( !iev )  {
            Q_ASSERT( iev );
            return;
        }

        ImageRequest* request = iev->loadInfo();

        QMutexLocker requestLocker( &m_lock );
        const bool requestStillNeeded = m_loadList.isRequestStillValid( request );
        m_loadList.removeRequest(request);
        m_currentLoading.remove( request );
        requestLocker.unlock();

        QImage image = iev->image();
        if ( !request->loadedOK() ) {
            if ( m_brokenImage.size() != request->size() ) {
                // we can ignore the krazy warning here because we have a valid fallback
                QIcon brokenFileIcon = QIcon::fromTheme( QLatin1String("file-broken") ); // krazy:exclude=iconnames
                if ( brokenFileIcon.isNull() ) {
                    brokenFileIcon = QIcon::fromTheme( QLatin1String("image-x-generic") );
                }
                m_brokenImage = brokenFileIcon.pixmap( request->size() ).toImage();
            }

            image = m_brokenImage;
        }

        if ( request->isThumbnailRequest() )
            ImageManager::ThumbnailCache::instance()->insert( request->databaseFileName(), image );


        if ( requestStillNeeded && request->client() ) {
            request->client()->pixmapLoaded(request, image);
        }
        delete request;
    }
    else if ( ev->type() == CANCELEVENTID ) {
        CancelEvent* cancelEvent = dynamic_cast<CancelEvent*>(ev);
        cancelEvent->request()->client()->requestCanceled();
    }
}
// vi:expandtab:tabstop=4 shiftwidth=4:
