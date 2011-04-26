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
#include "RequestQueue.h"
#include "ImageRequest.h"
#include "ImageClient.h"
#include "CancelEvent.h"
#include <QApplication>
#include "Manager.h"

bool ImageManager::RequestQueue::addRequest( ImageRequest* request )
{
    if ( _uniquePending.contains( request ) ) {
        // We have this very same request already in the queue. Ignore this one.
        delete request;
        return false;
    }

    _queues[ request->priority() ].enqueue( request );
    _uniquePending.insert( request );

    if ( request->client() )
        _activeRequests.insert( request );

    return true;
}

ImageManager::ImageRequest* ImageManager::RequestQueue::popNext()
{
    QueueType::iterator it = _queues.end(); // _queues is initialized to non-zero size
    do {
        --it;
        while ( ! it->empty() ) {
            ImageRequest* request = it->dequeue();

            if ( ! request->stillNeeded() ) {
                removeRequest( request );
                request->setLoadedOK( false );
                CancelEvent* event = new CancelEvent( request );
                QApplication::postEvent( Manager::instance(),  event );
            } else {
                _uniquePending.remove( request );
                return request;
            }
        }
    } while ( it != _queues.begin() );

    return NULL;
}

void ImageManager::RequestQueue::cancelRequests( ImageClient* client, StopAction action )
{
    // remove from active map
    for( QSet<ImageRequest*>::const_iterator it = _activeRequests.begin(); it != _activeRequests.end(); ) {
        ImageRequest* request = *it;
        ++it; // We need to increase it before removing the element.
        if ( client == request->client() && ( action == StopAll || ( request->priority() < ThumbnailVisible ) ) ) {
            _activeRequests.remove( request );
            // active requests are not deleted - they might already have been
            // popNext()ed and are being processed. They will be deleted
            // in Manger::customEvent().
        }
    }

    for ( QueueType::iterator qit = _queues.begin(); qit != _queues.end(); ++qit ) {
        for ( QQueue<ImageRequest*>::iterator it = qit->begin(); it != qit->end(); /* no increment here */) {
            ImageRequest* request = *it;
            if ( request->client() == client && ( action == StopAll || request->priority() < ThumbnailVisible ) ) {
                it = qit->erase( it );
                _uniquePending.remove( request );
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
    _activeRequests.remove( request );
    _uniquePending.remove( request );
}

ImageManager::RequestQueue::RequestQueue()
{
    for ( int i = 0; i < LastPriority; ++i )
        _queues.append( QQueue<ImageRequest*>() );
}


#include "RequestQueue.moc"
