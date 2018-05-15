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
#include "ImageClientInterface.h"
#include "CancelEvent.h"
#include <QApplication>
#include "AsyncLoader.h"

bool ImageManager::RequestQueue::addRequest( ImageRequest* request )
{
    const ImageRequestReference ref(request);
    if ( m_uniquePending.contains( ref ) ) {
        // We have this very same request already in the queue. Ignore this one.
        delete request;
        return false;
    }

    m_queues[ request->priority() ].enqueue( request );
    m_uniquePending.insert( ref );

    if ( request->client() )
        m_activeRequests.insert( request );

    return true;
}

ImageManager::ImageRequest* ImageManager::RequestQueue::popNext()
{
    QueueType::iterator it = m_queues.end(); // m_queues is initialized to non-zero size
    do {
        --it;
        while ( ! it->empty() ) {
            ImageRequest* request = it->dequeue();

            if ( ! request->stillNeeded() ) {
                removeRequest( request );
                request->setLoadedOK( false );
                CancelEvent* event = new CancelEvent( request );
                QApplication::postEvent( AsyncLoader::instance(),  event );
            } else {
                const ImageRequestReference ref(request);
                m_uniquePending.remove( ref );
                return request;
            }
        }
        if ( AsyncLoader::instance()->isExiting() )
            return new ImageRequest(true);
    } while ( it != m_queues.begin() );

    return nullptr;
}

void ImageManager::RequestQueue::cancelRequests( ImageClientInterface* client, StopAction action )
{
    // remove from active map
    for( QSet<ImageRequest*>::const_iterator it = m_activeRequests.begin(); it != m_activeRequests.end(); ) {
        ImageRequest* request = *it;
        ++it; // We need to increase it before removing the element.
        if ( client == request->client() && ( action == StopAll || ( request->priority() < ThumbnailVisible ) ) ) {
            m_activeRequests.remove( request );
            // active requests are not deleted - they might already have been
            // popNext()ed and are being processed. They will be deleted
            // in Manger::customEvent().
        }
    }

    for ( QueueType::iterator qit = m_queues.begin(); qit != m_queues.end(); ++qit ) {
        for ( QQueue<ImageRequest*>::iterator it = qit->begin(); it != qit->end(); /* no increment here */) {
            ImageRequest* request = *it;
            if ( request->client() == client && ( action == StopAll || request->priority() < ThumbnailVisible ) ) {
                it = qit->erase( it );
                const ImageRequestReference ref(request);
                m_uniquePending.remove( ref );
                delete request;
            } else {
                ++it;
            }
        }
    }
}

bool ImageManager::RequestQueue::isRequestStillValid( ImageRequest* request )
{
    return m_activeRequests.contains( request );
}

void ImageManager::RequestQueue::removeRequest( ImageRequest* request )
{
    const ImageRequestReference ref(request);
    m_activeRequests.remove( request );
    m_uniquePending.remove( ref );
}

ImageManager::RequestQueue::RequestQueue()
{
    for ( int i = 0; i < LastPriority; ++i )
        m_queues.append( QQueue<ImageRequest*>() );
}

// vi:expandtab:tabstop=4 shiftwidth=4:
