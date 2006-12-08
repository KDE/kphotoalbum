/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
#include "RequestQueue.h"
#include "ImageRequest.h"
#include <qtimer.h>
void ImageManager::RequestQueue::addRequest( ImageRequest* request )
{
    for( QValueList<ImageRequest*>::ConstIterator pendingIt = _pendingRequests.begin();
         pendingIt != _pendingRequests.end(); ++pendingIt ) {
        if ( *request == *(*pendingIt) )
            return;
    }

    if ( request->priority() )
        _pendingRequests.prepend( request );
    else
        _pendingRequests.append( request );

    if ( request->client() )
        _activeRequests.insert( request );
}

ImageManager::ImageRequest* ImageManager::RequestQueue::popNext()
{
    while ( _pendingRequests.count() != 0 ) {
        ImageRequest* request = _pendingRequests.first();
        _pendingRequests.pop_front();

        if ( !request->stillNeeded() ) {
            _activeRequests.remove( request );
            delete request;
        }
        else
            return request;
    }

    return 0;
}

void ImageManager::RequestQueue::cancelRequests( ImageClient* client, StopAction action )
{
    // remove from active map
    for( Set<ImageRequest*>::Iterator it = _activeRequests.begin(); it != _activeRequests.end(); ) {
        ImageRequest* request = *it;
        ++it; // We need to increase it before removing the element.
        if ( client == request->client() && ( action == StopAll || !request->priority() ) )
            _activeRequests.remove( request );
    }

    for( QValueList<ImageRequest*>::Iterator it = _pendingRequests.begin(); it != _pendingRequests.end(); ) {
        ImageRequest* request = *it;
        ++it;
        if ( request->client() == client && ( action == StopAll || !request->priority() ) )
            _pendingRequests.remove( request );
    }
}

bool ImageManager::RequestQueue::isRequestStillValid( ImageRequest* request )
{
    return _activeRequests.contains( request );
}

void ImageManager::RequestQueue::removeRequest( ImageRequest* request )
{
    _activeRequests.remove( request );
}

void ImageManager::RequestQueue::print()
{
    qDebug("%d %d", _activeRequests.count(), _pendingRequests.count() );
}

ImageManager::RequestQueue::RequestQueue()
{
#if 0
    QTimer* timer = new QTimer( this );
    timer->start( 200 );
    connect( timer, SIGNAL( timeout() ), this, SLOT( print() ));
#endif
}


#include "RequestQueue.moc"
