#include "ImageProvider.h"
#include "RemoteInterface.h"
#include <QDebug>

ImageProvider::ImageProvider()
    : QQuickImageProvider(QQmlImageProviderBase::Image)
{
}

QImage ImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    auto image = RemoteControl::RemoteInterface::instance().m_homeImage;
    qDebug() << id << image.size();
    *size = image.size();
    return image;
}
