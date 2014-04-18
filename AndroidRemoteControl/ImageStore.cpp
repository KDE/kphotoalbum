#include "ImageStore.h"
#include "RemoteInterface.h"
#include "Settings.h"

#include <QTimer>
#include "RemoteImage.h"
#include <QMutexLocker>


namespace RemoteControl {

ImageStore&ImageStore::instance()
{
    static ImageStore instance;
    return instance;
}

ImageStore::ImageStore()
{
    connect(&Settings::instance(), &Settings::thumbnailSizeChanged, this, &ImageStore::reset);
    connect(&Settings::instance(), &Settings::categoryItemSizeChanged, this, &ImageStore::reset);
}

void ImageStore::requestImage(RemoteImage* client, int imageId, const QSize& size, ViewType type)
{
    // This code is executed from paint, which is on the QML thread, we therefore need to get it on the GUI thread
    // where out TCPSocket is located.
    QTimer* timer = new QTimer;
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [imageId,size,type,timer,client, this] () {
        ThumbnailRequest request(imageId, size, type);

        RemoteInterface::instance().sendCommand(request);

        RequestType key = qMakePair(imageId,type);
        m_requestMap.insert(key, client);
        m_reverseRequestMap.insert(client, key);

        connect(client, &QObject::destroyed, this, &ImageStore::clientDeleted);
        timer->deleteLater();
    });
    timer->start(0);
}

void ImageStore::updateImage(int imageId, const QImage& image, const QString& label, ViewType type)
{
    QMutexLocker locker(&m_mutex);
    RequestType key = qMakePair(imageId,type);
    if (m_requestMap.contains(key)) {
        m_imageMap[key] = image;
        m_requestMap[key]->update();
        m_labelMap[imageId] = label;
        m_requestMap[key]->setLabel(label);
    }
}

QImage RemoteControl::ImageStore::image(RemoteImage* client, int imageId, const QSize& size, ViewType type)
{
    // This method is call from the painting thread.
    QMutexLocker locker(&m_mutex);

    if (m_imageMap.contains(qMakePair(imageId,type)))
        return m_imageMap[qMakePair(imageId,type)];
    else {
        requestImage(client, imageId,size,type);
        QImage image(size, QImage::Format_RGB32);
        image.fill(Qt::white);
        return image;
    }
}

QString ImageStore::label(int imageId) const
{
    return m_labelMap[imageId];
}

void RemoteControl::ImageStore::reset()
{
    QList<RemoteImage*> keys = m_reverseRequestMap.keys();
    m_imageMap.clear();
    m_reverseRequestMap.clear();
    m_requestMap.clear();
}

void ImageStore::clientDeleted()
{
    RemoteImage* remoteImage = static_cast<RemoteImage*>(sender());

    QMutexLocker locker(&m_mutex);
    if (m_reverseRequestMap.contains(remoteImage)) {
        RequestType key = m_reverseRequestMap[remoteImage];
        m_reverseRequestMap.remove(remoteImage);
        m_requestMap.remove(key);

        // FIXME: I'm sending cancel's for images that might already have got their answer
        if (key.second == ViewType::Images)
            RemoteInterface::instance().sendCommand(CancelRequestCommand(key.first, key.second));
    }
}

} // namespace RemoteControl
