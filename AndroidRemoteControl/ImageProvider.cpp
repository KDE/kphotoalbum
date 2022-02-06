/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

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

void ImageProvider::setImages(const RemoteControl::StaticImageResult &images)
{
    m_images = images;
    m_ready = true;
    emit imagesChanged();
}

// The QtQuick.Controls.Menu items does unfortunately not have an icon property where I can provide a QImage, so I need this interface.
QImage ImageProvider::requestImage(const QString &id, QSize *size, const QSize & /*requestedSize*/)
{
    auto image = [&] {
        if (id == "home")
            return m_images.homeIcon;
        else if (id == "info")
            return m_images.info;
        else if (id == "slideShow")
            return m_images.slideShow;
        else if (id == "search")
            return m_images.discoverIcon;
        else if (id == "kphotoalbum")
            return m_images.kphotoalbumIcon;
        else if (id == "slideShowSpeed")
            return m_images.slideShowSpeed;
        else if (id == "stopSlideShow")
            return m_images.stop;
        Q_UNREACHABLE();
    }();
    *size = image.size();
    return image;
}

const QImage &ImageProvider::homeIcon() const
{
    return m_images.homeIcon;
}

const QImage &ImageProvider::kphotoalbumIcon() const
{
    return m_images.kphotoalbumIcon;
}

const QImage &ImageProvider::discoverIcon() const
{
    return m_images.discoverIcon;
}

const QImage &ImageProvider::info() const
{
    return m_images.info;
}

const QImage &ImageProvider::slideShow() const
{
    return m_images.slideShow;
}
