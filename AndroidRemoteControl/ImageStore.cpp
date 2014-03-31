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
    m_imageMap[fileName] = image;
    emit imageUpdated(fileName);
}

QImage RemoteControl::ImageStore::image(const QString& fileName) const
{
    if (m_imageMap.contains(fileName))
        return m_imageMap[fileName];
    else {
        const int size = Settings::instance().thumbnailSize();
        RemoteInterface::instance().sendCommand(ThumbnailRequest(fileName, size, size));
        QImage image(size, size, QImage::Format_RGB32);
        image.fill(Qt::white);
        return image;
    }
}

void RemoteControl::ImageStore::reset()
{
    const QStringList fileNames = m_imageMap.keys();
    m_imageMap.clear();
    for (const QString& fileName : fileNames) {
        emit imageUpdated(fileName); // While clear the image and request it a new
    }
}

} // namespace RemoteControl
