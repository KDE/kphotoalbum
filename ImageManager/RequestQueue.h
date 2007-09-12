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
#ifndef REQUESTQUEUE_H
#define REQUESTQUEUE_H

#include "StopAction.h"
#include "Utilities/Set.h"
#include "ImageManager/ImageRequest.h"
#include <qvaluelist.h>
#include <qobject.h>

namespace ImageManager
{
using Utilities::Set;

class ImageRequest;
class ImageClient;

// RequestQueue for ImageRequests. Non-synchronized, locking has to be
// provided by the user.
class RequestQueue :public QObject
{
    Q_OBJECT

public:
    RequestQueue();

    // Add a new request to the input queue. If this is a priority request,
    // this will be the next request picked up by popNext(), otherwise
    // the request is appended to the end of the input queue.
    void addRequest( ImageRequest* request );

    // Return the next needed ImageRequest from the queue or NULL if there
    // is none.
    ImageRequest* popNext();

    // Remove all pending requests from the given client.
    void cancelRequests( ImageClient* client, StopAction action );

    bool isRequestStillValid( ImageRequest* request );
    void removeRequest( ImageRequest* );

protected slots:
    void print();

private: 
    // A Reference to a ImageRequest with value semantic.
    // This only stores the pointer to an ImageRequest object but behaves
    // regarding the less-than and equals-operator like the object.
    // This allows to store ImageRequests with value-semantic in a Set.
    class ImageRequestReference {
    public:
        ImageRequestReference() : _ptr(0) {}
        ImageRequestReference(const ImageRequestReference& other) 
            : _ptr(other._ptr) {}
        ImageRequestReference(ImageRequest* ptr) : _ptr(ptr) {}
        
        bool operator<(const ImageRequestReference &other) const {
            return *_ptr < *other._ptr;
        }
        bool operator==(const ImageRequestReference &other) const {
            return *_ptr == *other._ptr;
        }

    private:
        ImageRequest * _ptr;
    };

    // Input queue of pending input requests.
    QValueList<ImageRequest*> _pendingRequests;

    // Set of unique requests currently pending; used to discard the exact
    // same requests.
    Set<ImageRequestReference> _uniquePending;

    // All active requests that have a client
    Set<ImageRequest*> _activeRequests;
};

}

#endif /* REQUESTQUEUE_H */

