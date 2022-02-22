/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ImageStore.h"
#include "RemoteImage.h"
#include "RemoteInterface.h"
#include "Settings.h"
#include <QMutexLocker>
#include <QTimer>

namespace RemoteControl
{

ImageStore &ImageStore::instance()
{
    static ImageStore instance;
    return instance;
}

ImageStore::ImageStore()
{
    connect(&Settings::instance(), &Settings::thumbnailSizeChanged, this, &ImageStore::reset);
    connect(&Settings::instance(), &Settings::categoryItemSizeChanged, this, &ImageStore::reset);
}

void ImageStore::requestImage(RemoteImage *client, ImageId imageId, const QSize &size, ViewType type)
{
    // This method is call from the painting thread.
    QMutexLocker locker(&m_mutex);

    // This code is executed from paint, which is on the QML thread, we therefore need to get it on the GUI thread
    // where out TCPSocket is located.
    QTimer *timer = new QTimer;
    timer->setSingleShot(true);

    // There seems to be a path through QML where the client is deleted right after this request is send,
    // therefore, have client as the third parameter to the connect below, as that will prevent the request in that setup.
    connect(timer, &QTimer::timeout, client, [imageId, size, type, timer, client, this]() {
        ThumbnailRequest request(imageId, size, type);

        RemoteInterface::instance().sendCommand(request);

        RequestType key = qMakePair(imageId, type);
        m_requestMap.insert(key, client);
        m_reverseRequestMap.insert(client, key);

        connect(client, &QObject::destroyed, this, &ImageStore::clientDeleted);
        timer->deleteLater();
    });
    timer->start(0);
}

void ImageStore::setImageDates(const QHash<ImageId, QDate> &imageDates)
{
    m_imageDates = imageDates;
}

QDate ImageStore::date(ImageId id) const
{
    return m_imageDates.value(id);
}

void ImageStore::updateImage(ImageId imageId, const QImage &image, const QString &label, ViewType type)
{
    // FIXME are there even threads involved?!
    QMutexLocker locker(&m_mutex);
    RequestType key = qMakePair(imageId, type);
    if (m_requestMap.contains(key)) {
        RemoteImage *client = m_requestMap[key];

        client->setImage(image);
        client->setLabel(label);

        m_requestMap.remove(key);
        m_reverseRequestMap.remove(client);
    }
}

void RemoteControl::ImageStore::reset()
{
    QList<RemoteImage *> keys = m_reverseRequestMap.keys();
    m_reverseRequestMap.clear();
    m_requestMap.clear();
}

void ImageStore::clientDeleted()
{
    RemoteImage *remoteImage = static_cast<RemoteImage *>(sender());

    QMutexLocker locker(&m_mutex);
    if (m_reverseRequestMap.contains(remoteImage)) {
        RequestType key = m_reverseRequestMap[remoteImage];
        m_reverseRequestMap.remove(remoteImage);
        m_requestMap.remove(key);

        if (key.second == ViewType::Thumbnails)
            RemoteInterface::instance().sendCommand(ThumbnailCancelRequest(key.first, key.second));
    }
}

} // namespace RemoteControl

#include "moc_ImageStore.cpp"
