#include "ImageStore.h"
#include "RemoteInterface.h"

namespace RemoteControl {

ImageStore&ImageStore::instance()
{
    static ImageStore instance;
    return instance;
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
        RemoteInterface::instance().sendCommand(ThumbnailRequest(fileName));
        return {};
        //        QImage image(200, 200, QImage::Format_RGB32);
        //        image.fill(Qt::blue);
        //        return image;
    }
}

} // namespace RemoteControl
