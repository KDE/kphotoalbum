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

#include "Manager.h"
#include <KIcon>
#include "ThumbnailCache.h"
#include "ImageLoader.h"
#include "ImageManager/ImageClient.h"
#include "Utilities/Util.h"
#include "VideoManager.h"

#include <kurl.h>
#include <qpixmapcache.h>
#include "ImageEvent.h"
#include "CancelEvent.h"

ImageManager::Manager* ImageManager::Manager::_instance = 0;

// -- Manager --

ImageManager::Manager* ImageManager::Manager::instance()
{
    if ( !_instance )  {
        _instance = new Manager;
        _instance->init();
    }

    return _instance;
}

// We need this as a separate method as the _instance variable will otherwise not be initialized
// corrected before the thread starts.
void ImageManager::Manager::init()
{
    ImageLoader* imageLoader;
    int cores = qMax( 2, QThread::idealThreadCount() );

    for ( int i = 0; i < cores; ++i) {
        imageLoader = new ImageLoader();
        imageLoader->start( QThread::LowPriority );
    }
}

void ImageManager::Manager::load( ImageRequest* request )
{
    if ( Utilities::isVideo( request->fileName() ) )
        loadVideo( request );
    else
        loadImage( request );
}

void ImageManager::Manager::loadVideo( ImageRequest* request)
{
    VideoManager::instance().request( request );
}

void ImageManager::Manager::loadImage( ImageRequest* request )
{
    QMutexLocker dummy( &_lock );
    QSet<ImageRequest*>::const_iterator req = _currentLoading.find( request );
    if ( req != _currentLoading.end() && _loadList.isRequestStillValid( request ) ) {
        // The last part of the test above is needed to not fail on a race condition from AnnotationDialog::ImagePreview, where the preview
        // at startup request the same image numerous time (likely from resize event).
        Q_ASSERT ( *req != request);
        delete request;

        return; // We are currently loading it, calm down and wait please ;-)
    }

    if (_loadList.addRequest( request ))
        _sleepers.wakeOne();
}

void ImageManager::Manager::stop( ImageClient* client, StopAction action )
{
    // remove from pending map.
    _lock.lock();
    _loadList.cancelRequests( client, action );
    _lock.unlock();

    VideoManager::instance().stop( client, action );
}

ImageManager::ImageRequest* ImageManager::Manager::next()
{
    QMutexLocker dummy(&_lock );
    ImageRequest* request = 0;
    while ( !( request = _loadList.popNext() ) )
        _sleepers.wait( &_lock );
    _currentLoading.insert( request );

    return request;
}

void ImageManager::Manager::customEvent( QEvent* ev )
{
    if ( ev->type() == ImageEventID )  {
        ImageEvent* iev = dynamic_cast<ImageEvent*>( ev );
        if ( !iev )  {
            Q_ASSERT( iev );
            return;
        }

        ImageRequest* request = iev->loadInfo();
        QImage image = iev->image();

        ImageClient* client = 0;
        QString fileName;
        QSize size;
        QSize fullSize;
        int angle = 0;
        bool loadedOK = request->loadedOK();

        _lock.lock();
        if ( _loadList.isRequestStillValid( request ) )  {
            // If it is not in the map, then it has been canceled (though ::stop) since the request.
            client = request->client();
            fileName = request->fileName();
            size = QSize(request->width(), request->height() );
            fullSize = request->fullSize();
            angle = request->angle();
        }

        if ( request->isThumbnailRequest() ) {
            if ( !request->loadedOK() ) {
                // PENDING(blackie) This stinks! It looks bad, but I don't have more energy to fix it.
                KIcon icon( QString::fromLatin1( "file-broken" ) );
                QPixmap pix = icon.pixmap( icon.actualSize( QSize( request->width(), request->height() ) ) );
                image = pix.toImage();
            }

            ImageManager::ThumbnailCache::instance()->insert( request->fileName(), image );
        }

        _loadList.removeRequest(request);
        _currentLoading.remove( request );
        delete request;

        _lock.unlock();
        if ( client ) {
            client->pixmapLoaded( fileName, size, fullSize, angle, image, loadedOK);
        }
    }
    else if ( ev->type() == CANCELEVENTID ) {
        CancelEvent* cancelEvent = dynamic_cast<CancelEvent*>(ev);
        cancelEvent->request()->client()->requestCanceled();
    }
}

#include "Manager.moc"
