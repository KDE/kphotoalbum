/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "imagemanager.h"
#include "imageloader.h"
#include "options.h"
#include "imageclient.h"
#include <qdatetime.h>
#include <qmutex.h>
#include <qapplication.h>

ImageManager* ImageManager::_instance = 0;

/**
   This class is responsible for loading icons in a separate thread.
   I tried replacing this with KIO:PreviewJob, but it had a fwe drawbacks:
   1) It stored images in one centeral directory - many would consider this
      a feature, but I consider it a drawback, as it makes it impossible to
      just bring your thumbnails when bringing your database, but not having
      the capasity on say your laptop to bring all your images.
   2) It failed to load a number of images, that this ImageManager load
      just fine.
   3) Most important, it did not allow loading only thumbnails when the
      image themself weren't available.
*/
ImageManager::ImageManager() :_currentLoading(0)
{
    _clientList.resize( 9973 /* a large prime */ );
}

// We need this as a separate method as the _instance variable will otherwise not be initialized
// corrected before the thread starts.
void ImageManager::init()
{
    ImageLoader* imageLoader = new ImageLoader( &_sleepers );
    imageLoader->start();
}

void ImageManager::load( ImageRequest* request )
{
    QMutexLocker dummy( &_lock );
    if ( _currentLoading && _currentLoading->fileName() == request->fileName() && _currentLoading->client() == request->client() &&
         _currentLoading->width() == request->width() && _currentLoading->height() == request->height() ) {
        return; // We are currently loading it, calm down and wait please ;-)
    }

    // Delete other request for the same file from the same client
    for( QValueList<ImageRequest*>::Iterator it = _loadList.begin(); it != _loadList.end(); ) {
        if ( (*it)->fileName() == request->fileName() && (*it)->client() == request->client() && (*it)->width() == request->width() && (*it)->height() == request->height()) {
            return; // This image is already in the queue
        }
        else
            ++it;
    }

    if ( request->priority() )
        _loadList.prepend( request );
    else
        _loadList.append( request );

    if ( request->client() )
        _clientList.insert( request, (void*)0x01 /*something different from 0x0 */ );

    _sleepers.wakeOne();
}

ImageRequest* ImageManager::next()
{
    QMutexLocker dummy(&_lock );
    ImageRequest* request = 0;
    while ( _loadList.count() != 0 ) {
        request = _loadList.first();
        _loadList.pop_front();
        // qApp->lock(); // stillNeeded are likely to do a GUI operation - this is on a thread, lock qApp!
        if ( !request->stillNeeded() ) {
            _clientList.remove( request );
            delete request;
            request = 0;
        }
        else
            break;
        // qApp->unlock();
    }
    _currentLoading = request;
    return request;
}

void ImageManager::customEvent( QCustomEvent* ev )
{
    if ( ev->type() == 1001 )  {
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
        int angle;
        bool loadedOK;

        _lock.lock();
        if ( _clientList.find( request ) != 0 )  {
            // If it is not in the map, then it has been canceled (though ::stop) since the request.
            client = request->client();
            fileName = request->fileName();
            size = QSize(request->width(), request->height() );
            fullSize = request->fullSize();
            angle = request->angle();
            loadedOK = request->loadedOK();

            _clientList.remove(request);
            if ( _currentLoading == request )
                _currentLoading = 0;
            delete request;
        }
        _lock.unlock();
        if ( client )
            client->pixmapLoaded( fileName, size, fullSize, angle, image, loadedOK );
    }
}

ImageEvent::ImageEvent( ImageRequest* request, const QImage& image )
    : QCustomEvent( 1001 ), _request( request ),  _image( image )
{
    // We would like to use QDeepCopy, but that results in multiple
    // individual instances on the GUI thread, which is kind of real bad
    // when  the image is like 40Mb large.
    _image.detach();
}

ImageRequest* ImageEvent::loadInfo()
{
    return _request;
}

ImageManager* ImageManager::instance()
{
    if ( !_instance )  {
        _instance = new ImageManager;
        _instance->init();
    }

    return _instance;
}

void ImageManager::stop( ImageClient* client, StopAction action )
{
    // remove from active map
    for( QPtrDictIterator<void> it(_clientList); it.current(); ) {
        ImageRequest* request = static_cast<ImageRequest*>(it.currentKey());
        ++it; // We need to increase it before removing the element.
        if ( client == request->client() && ( action == StopAll || !request->priority() ) )
            _clientList.remove( request );
    }

    // remove from pending map.
    _lock.lock();
    for( QValueList<ImageRequest*>::Iterator it = _loadList.begin(); it != _loadList.end(); ) {
        ImageRequest* request = *it;
        ++it;
        if ( request->client() == client && ( action == StopAll || !request->priority() ) )
            _loadList.remove( request );
    }
    _lock.unlock();
}

QImage ImageEvent::image()
{
    return _image;
}

#include "imagemanager.moc"
