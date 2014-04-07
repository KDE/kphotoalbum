#include "ImageStore.h"
#include "RemoteInterface.h"
#include "Settings.h"
#include <QDebug>

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

void ImageStore::updateImage(const QString& fileName, const QImage& image)
{
    // PENDING(blackie) Information about image type should come from the remote site!
    ViewType type = ((image.size().width() == Settings::instance().thumbnailSize() ||
                     image.size().height() == Settings::instance().thumbnailSize()) ? ViewType::Thumbnail : ViewType::ImageView);
    m_imageMap[qMakePair(fileName,type)] = image;
    emit imageUpdated(fileName,type);
}

QImage RemoteControl::ImageStore::image(const QString& fileName, const QSize& size, ViewType type) const
{
    if (m_imageMap.contains(qMakePair(fileName,type)))
        return m_imageMap[qMakePair(fileName,type)];
    else {
        RemoteInterface::instance().sendCommand(ThumbnailRequest(fileName, size, type));
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
