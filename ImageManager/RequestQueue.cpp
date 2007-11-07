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

#include <kdebug.h>

void ImageManager::RequestQueue::addRequest( ImageRequest* request )
{
    if ( _uniquePending.contains( request ) && ! request->priority() ) {
        delete request;
        return;
    }
 
    _uniquePending.insert( request );
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
        _uniquePending.erase( request );

        if ( !request->stillNeeded() ) {
            _activeRequests.erase( request );
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
    for( Set<ImageRequest*>::const_iterator it = _activeRequests.begin(); it != _activeRequests.end(); ) {
        ImageRequest* request = *it;
        ++it; // We need to increase it before removing the element.
        if ( client == request->client() && ( action == StopAll || !request->priority() ) ) {
            _activeRequests.erase( request );
            // active requests are not deleted - they might already have been
            // popNext()ed and are being processed. They will be deleted
            // in Manger::customEvent().
        }
    }

    for( QValueList<ImageRequest*>::Iterator it = _pendingRequests.begin(); it != _pendingRequests.end(); ) {
        ImageRequest* request = *it;
        ++it;
        if ( request->client() == client && ( action == StopAll || !request->priority() ) ) {
            _pendingRequests.remove( request );
            _uniquePending.erase( request );
            delete request;
        }
    }
}

bool ImageManager::RequestQueue::isRequestStillValid( ImageRequest* request )
{
    return _activeRequests.contains( request );
}

void ImageManager::RequestQueue::removeRequest( ImageRequest* request )
{
    _activeRequests.erase( request );
}

void ImageManager::RequestQueue::print()
{
    kdDebug() << "**************************************" << endl;
    kdDebug() << "Active: " << _activeRequests.size() << ", pending: " << _pendingRequests.count() << endl;
    kdDebug() << "Active:" << endl;
    for (Set<ImageRequest*>::const_iterator it = _activeRequests.begin(); it != _activeRequests.end(); ++it ) {
        kdDebug() << (*it)->fileName() << " " <<  (*it)->width() << "x" <<  (*it)->height() << endl;
    }
    kdDebug() << "pending:" << endl;
    for (QValueList<ImageRequest*>::const_iterator it = _pendingRequests.begin(); it != _pendingRequests.end(); ++it ) {
        kdDebug() << (*it)->fileName() << " " <<  (*it)->width() << "x" <<  (*it)->height() << endl;
    }
}

ImageManager::RequestQueue::RequestQueue()
{
#if 0
    QTimer* timer = new QTimer( this );
    timer->start( 500 );
    connect( timer, SIGNAL( timeout() ), this, SLOT( print() ));
#endif
}


#include "RequestQueue.moc"
