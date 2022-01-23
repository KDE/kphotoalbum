#include "ImageProvider.h"
#include "RemoteInterface.h"
#include <QDebug>

ImageProvider::ImageProvider()
    : QQuickImageProvider(QQmlImageProviderBase::Image)
{
}

ImageProvider &ImageProvider::instance()
{
    static ImageProvider instance;
    return instance;
}

QImage ImageProvider::requestImage(const QString &id, QSize *size, const QSize & /*requestedSize*/)
{
    auto image = [&] {
        if (id == "home")
            return RemoteControl::RemoteInterface::instance().m_homeImage;
        else if (id == "info")
            return m_info;
        else if (id == "slideShow")
            return m_slideShow;
        else if (id == "search")
            return m_search;
        Q_UNREACHABLE();
    }();
    *size = image.size();
    return image;
}
