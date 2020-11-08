/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "RequestQueue.h"

#include "AsyncLoader.h"
#include "CancelEvent.h"
#include "ImageClientInterface.h"
#include "ImageRequest.h"

#include <QApplication>

bool ImageManager::RequestQueue::addRequest(ImageRequest *request)
{
    const ImageRequestReference ref(request);
    if (m_uniquePending.contains(ref)) {
        // We have this very same request already in the queue. Ignore this one.
        delete request;
        return false;
    }

    m_queues[request->priority()].enqueue(request);
    m_uniquePending.insert(ref);

    if (request->client())
        m_activeRequests.insert(request);

    return true;
}

ImageManager::ImageRequest *ImageManager::RequestQueue::popNext()
{
    QueueType::iterator it = m_queues.end(); // m_queues is initialized to non-zero size
    do {
        --it;
        while (!it->empty()) {
            ImageRequest *request = it->dequeue();

            if (!request->stillNeeded()) {
                removeRequest(request);
                request->setLoadedOK(false);
                CancelEvent *event = new CancelEvent(request);
                QApplication::postEvent(AsyncLoader::instance(), event);
            } else {
                const ImageRequestReference ref(request);
                m_uniquePending.remove(ref);
                return request;
            }
        }
        if (AsyncLoader::instance()->isExiting())
            return new ImageRequest(true);
    } while (it != m_queues.begin());

    return nullptr;
}

void ImageManager::RequestQueue::cancelRequests(ImageClientInterface *client, StopAction action)
{
    // remove from active map
    for (QSet<ImageRequest *>::const_iterator it = m_activeRequests.begin(); it != m_activeRequests.end();) {
        ImageRequest *request = *it;
        ++it; // We need to increase it before removing the element.
        if (client == request->client() && (action == StopAll || (request->priority() < ThumbnailVisible))) {
            m_activeRequests.remove(request);
            // active requests are not deleted - they might already have been
            // popNext()ed and are being processed. They will be deleted
            // in Manger::customEvent().
        }
    }

    for (QueueType::iterator qit = m_queues.begin(); qit != m_queues.end(); ++qit) {
        for (QQueue<ImageRequest *>::iterator it = qit->begin(); it != qit->end(); /* no increment here */) {
            ImageRequest *request = *it;
            if (request->client() == client && (action == StopAll || request->priority() < ThumbnailVisible)) {
                it = qit->erase(it);
                const ImageRequestReference ref(request);
                m_uniquePending.remove(ref);
                delete request;
            } else {
                ++it;
            }
        }
    }
}

bool ImageManager::RequestQueue::isRequestStillValid(ImageRequest *request)
{
    return m_activeRequests.contains(request);
}

void ImageManager::RequestQueue::removeRequest(ImageRequest *request)
{
    const ImageRequestReference ref(request);
    m_activeRequests.remove(request);
    m_uniquePending.remove(ref);
}

ImageManager::RequestQueue::RequestQueue()
{
    for (int i = 0; i < LastPriority; ++i)
        m_queues.append(QQueue<ImageRequest *>());
}

// vi:expandtab:tabstop=4 shiftwidth=4:
