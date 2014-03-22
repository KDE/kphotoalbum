#include "RemoteImage.h"
#include "RemoteInterface.h"
#include <QPainter>

using namespace RemoteControl;

RemoteImage::RemoteImage(QQuickItem *parent) :
    QQuickPaintedItem(parent)
{
    connect(&RemoteInterface::instance(), &RemoteInterface::newImage, this, &RemoteImage::setImage);
    connect(&RemoteInterface::instance(), &RemoteInterface::connectionChanged, this, &RemoteImage::connectionChanged);
}

void RemoteImage::paint(QPainter* painter)
{
    painter->drawImage(0,0, m_image);
    //painter->drawImage(0,0, m_image.scaled(QSize(width(), height()), Qt::KeepAspectRatio));
}

bool RemoteImage::isConnected() const
{
    return RemoteInterface::instance().isConnected();
}

void RemoteImage::setImage(const QImage& image)
{
    m_image = image;
    update();
    setSize(image.size());
}
