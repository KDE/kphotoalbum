/* Copyright (C) 2003-2004 Jesper K. Pedersen <blackie@kde.org>

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
ImageManager::ImageManager()
{
}

// We need this as a separate method as the _instance variable will otherwise not be initialized
// corrected before the thread starts.
void ImageManager::init()
{
    _sleepers = new QWaitCondition(); // Is it necessary to load this using new?
    _lock = new QMutex(); // necessary with new?

    ImageLoader* imageLoader = new ImageLoader( _sleepers );
    imageLoader->start();
}

#ifdef TEMPORARILY_REMOVED
void ImageManager::load( const QString& fileName, ImageClient* client, int angle, int width, int height,
                         bool cache, bool priority )
{
    ImageRequest request( fileName, width, height, angle, client );
    request.setCache( cache );
    request.setPriority( priority );
    load( request );
}
#endif

void ImageManager::load( const ImageRequest& request )
{
    if ( _currentLoading.fileName() == request.fileName() && _currentLoading.client() == request.client() &&
         _currentLoading.width() == request.width() && _currentLoading.height() == request.height() ) {
        return; // We are currently loading it, calm down and wait please ;-)
    }

    _lock->lock();

    // Delete other request for the same file from the same client
    for( QValueList<ImageRequest>::Iterator it = _loadList.begin(); it != _loadList.end(); ) {
        if ( (*it).fileName() == request.fileName() && (*it).client() == request.client() && (*it).width() == request.width() && (*it).height() == request.height())
            it = _loadList.remove(it) ;
        else
            ++it;
    }
    if ( request.priority() )
        _loadList.prepend( request );
    else
        _loadList.append( request );
    _lock->unlock();
    if ( request.client() )
        _clientMap.insert( request, request.client() ); // PENDING(blackie) why do I need a map, couldn't I just go directly to the request?
    _sleepers->wakeOne();
}

ImageRequest ImageManager::next()
{
    _lock->lock();
    ImageRequest info;
    if ( _loadList.count() != 0 )  {
        info = _loadList.first();
        _loadList.pop_front();
    }
    _lock->unlock();
    _currentLoading = info;
    return info;
}

void ImageManager::customEvent( QCustomEvent* ev )
{
    if ( ev->type() == 1001 )  {
        ImageEvent* iev = dynamic_cast<ImageEvent*>( ev );
        if ( !iev )  {
            Q_ASSERT( iev );
            return;
        }

        ImageRequest li = iev->loadInfo();
        QImage image = iev->image();

        if ( _clientMap.contains( li ) )  {
            // If it is not in the map, then it has been deleted since the request.
            ImageClient* client = _clientMap[li];

            client->pixmapLoaded( li.fileName(), QSize(li.width(), li.height()), li.fullSize(), li.angle(), image, li.loadedOK() );
            _clientMap.remove(li);
        }
    }
}

ImageEvent::ImageEvent( ImageRequest info, const QImage& image )
    : QCustomEvent( 1001 ), _info( info ),  _image( image )
{
}

ImageRequest ImageEvent::loadInfo()
{
    return _info;
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
    for( QMapIterator<ImageRequest,ImageClient*> it= _clientMap.begin(); it != _clientMap.end(); ) {
        ImageRequest key = it.key();
        ImageClient* data = it.data();
        ++it; // We need to increase it before removing the element.
        if ( data == client && ( action == StopAll || !key.priority() ) )
            _clientMap.remove( key );
    }

    // remove from pending map.
    _lock->lock();
    for( QValueList<ImageRequest>::Iterator it = _loadList.begin(); it != _loadList.end(); ) {
        ImageRequest li = *it;
        ++it;
        if ( li.client() == client && ( action == StopAll || !li.priority() ) )
            _loadList.remove( li );
    }
    _lock->unlock();
}

QImage ImageEvent::image()
{
    return _image;
}

#include "imagemanager.moc"
