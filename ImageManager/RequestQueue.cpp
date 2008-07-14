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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "RequestQueue.h"
#include "ImageRequest.h"
#include <kdebug.h>

void ImageManager::RequestQueue::addRequest( ImageRequest* request )
{
    _pendingRequests[ request->priority() ].enqueue( request );

    if ( request->client() )
        _activeRequests.insert( request );
}

ImageManager::ImageRequest* ImageManager::RequestQueue::popNext()
{
    QueueType::iterator it = _pendingRequests.end(); // _pendingRequests is initialized to non-zero size
    do {
        --it;
        while ( ! it->empty() ) {
            ImageRequest* request = it->dequeue();

            if ( ! request->stillNeeded() ) {
                _activeRequests.erase( request );
                delete request;
            } else {
                return request;
            }
        } 
    } while ( it != _pendingRequests.begin() );
    return 0;
}

void ImageManager::RequestQueue::cancelRequests( ImageClient* client, StopAction action )
{
    // remove from active map
    for( Set<ImageRequest*>::const_iterator it = _activeRequests.begin(); it != _activeRequests.end(); ) {
        ImageRequest* request = *it;
        ++it; // We need to increase it before removing the element.
        if ( client == request->client() && ( action == StopAll || ( request->priority() < ThumbnailVisible ) ) ) {
            _activeRequests.erase( request );
            // active requests are not deleted - they might already have been
            // popNext()ed and are being processed. They will be deleted
            // in Manger::customEvent().
        }
    }

    for ( QueueType::iterator qit = _pendingRequests.begin(); qit != _pendingRequests.end(); ++qit ) {
        for ( QQueue<ImageRequest*>::iterator it = qit->begin(); it != qit->end(); /* no increment here */) {
            ImageRequest* request = *it;
            if ( request->client() == client && ( action == StopAll || request->priority() < ThumbnailVisible ) ) {
                it = qit->erase( it );
                delete request;
            } else {
                ++it;
            }
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

ImageManager::RequestQueue::RequestQueue()
{
    for ( int i = 0; i < LastPriority; ++i )
        _pendingRequests.append( QQueue<ImageRequest*>() );
}


#include "RequestQueue.moc"
