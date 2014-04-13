#include "ImageStore.h"
#include "RemoteInterface.h"
#include "Settings.h"

#include <QTimer>

namespace RemoteControl {

ImageStore&ImageStore::instance()
{
    static ImageStore instance;
    return instance;
}

ImageStore::ImageStore()
{
    connect(&Settings::instance(), &Settings::thumbnailSizeChanged, this, &ImageStore::reset);
}

void ImageStore::requestImage(const QString& fileName, const QSize& size, ViewType type) const
{
    // This code is executed from paint, which is on the QML thread, we therefore need to get it on the GUI thread
    // where out TCPSocket is located.
    QTimer* timer = new QTimer;
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [fileName,size,type,timer] () {
        ThumbnailRequest request(fileName, size, type);

        // The category is used when asking for category item images
        request.category = RemoteInterface::instance().currentCategory();
        RemoteInterface::instance().sendCommand(request);
        timer->deleteLater();
    });
    timer->start(0);
}

void ImageStore::updateImage(const QString& fileName, const QImage& image, ViewType type)
{
    if (type != ViewType::CategoryItems) {
        // PENDING(blackie) Information about image type should come from the remote site!
        type = ((image.size().width() == Settings::instance().thumbnailSize() ||
                 image.size().height() == Settings::instance().thumbnailSize()) ? ViewType::Thumbnails : ViewType::Images);
    }
    m_imageMap[qMakePair(fileName,type)] = image;
    emit imageUpdated(fileName,type);
}

QImage RemoteControl::ImageStore::image(const QString& fileName, const QSize& size, ViewType type) const
{
    qDebug("Request: %s type:%d has it:%d", qPrintable(fileName), type, m_imageMap.contains(qMakePair(fileName,type)));
    if (m_imageMap.contains(qMakePair(fileName,type)))
        return m_imageMap[qMakePair(fileName,type)];
    else {
        requestImage(fileName,size,type);
        QImage image(size, QImage::Format_RGB32);
        image.fill(Qt::white);
        return image;
    }
}

void RemoteControl::ImageStore::reset()
{
    QList<QPair<QString,ViewType>> keys = m_imageMap.keys();
    m_imageMap.clear();
    for (const auto& key : keys) {
        emit imageUpdated(key.first, key.second); // Clear the image and request it anew
    }
}

} // namespace RemoteControl
