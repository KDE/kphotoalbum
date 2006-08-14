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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "Manager.h"
#include "ImageLoader.h"
#include "ImageManager/ImageClient.h"
#include "Utilities/Util.h"
#include "VideoManager.h"

ImageManager::Manager* ImageManager::Manager::_instance = 0;

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
ImageManager::Manager::Manager() :_currentLoading(0)
{
}

// We need this as a separate method as the _instance variable will otherwise not be initialized
// corrected before the thread starts.
void ImageManager::Manager::init()
{
    ImageLoader* imageLoader = new ImageLoader( &_sleepers );
    imageLoader->start();
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
    if ( _currentLoading && *_currentLoading == *request ) {
        return; // We are currently loading it, calm down and wait please ;-)
    }

    _loadList.addRequest( request );

    _sleepers.wakeOne();
}

ImageManager::ImageRequest* ImageManager::Manager::next()
{
    QMutexLocker dummy(&_lock );
    _currentLoading = _loadList.popNext();
    return _currentLoading;
}

void ImageManager::Manager::customEvent( QCustomEvent* ev )
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
        if ( _loadList.isRequestStillValid( request ) )  {
            // If it is not in the map, then it has been canceled (though ::stop) since the request.
            client = request->client();
            fileName = request->fileName();
            size = QSize(request->width(), request->height() );
            fullSize = request->fullSize();
            angle = request->angle();
            loadedOK = request->loadedOK();

            _loadList.removeRequest(request);
            if ( _currentLoading == request )
                _currentLoading = 0;
            delete request;
        }
        _lock.unlock();
        if ( client )
            client->pixmapLoaded( fileName, size, fullSize, angle, image, loadedOK );
    }
}

ImageManager::ImageEvent::ImageEvent( ImageRequest* request, const QImage& image )
    : QCustomEvent( 1001 ), _request( request ),  _image( image )
{
    // We would like to use QDeepCopy, but that results in multiple
    // individual instances on the GUI thread, which is kind of real bad
    // when  the image is like 40Mb large.
    _image.detach();
}

ImageManager::ImageRequest* ImageManager::ImageEvent::loadInfo()
{
    return _request;
}

ImageManager::Manager* ImageManager::Manager::instance()
{
    if ( !_instance )  {
        _instance = new Manager;
        _instance->init();
    }

    return _instance;
}

void ImageManager::Manager::stop( ImageClient* client, StopAction action )
{
    // remove from pending map.
    _lock.lock();
    _loadList.cancelRequests( client, action );
    _lock.unlock();

    VideoManager::instance().stop( client, action );
}

QImage ImageManager::ImageEvent::image()
{
    return _image;
}

#include "Manager.moc"
