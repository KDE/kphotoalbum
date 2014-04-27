#include "RemoteImage.h"
#include "ImageStore.h"
#include <QPainter>
#include "RemoteInterface.h"
#include "Settings.h"

using namespace RemoteControl;

RemoteImage::RemoteImage(QQuickItem *parent) :
    QQuickPaintedItem(parent)
{
    connect(this, &RemoteImage::widthChanged, this, &RemoteImage::requestImage);
}

void RemoteImage::paint(QPainter* painter)
{
    painter->drawImage((width() - m_image.width()) / 2, height() - m_image.height(), m_image);
}

int RemoteImage::imageId() const
{
    return m_imageId;
}

QSize RemoteImage::size() const
{
    return QSize(width(),height());
}

void RemoteImage::setLabel(const QString& label)
{
    if (label != m_label) {
        m_label = label;
        emit labelChanged();
    }
}

void RemoteImage::setImage(const QImage& image)
{
    m_image = image;
    update();
}

void RemoteImage::setImageId(int imageId)
{
    if (m_imageId != imageId) {
        m_imageId = imageId;
        emit imageIdChanged();
    }
}

void RemoteImage::componentComplete()
{
    QQuickPaintedItem::componentComplete();
    requestImage();
}

void RemoteImage::requestImage()
{
    if (m_imageId == DISCOVERYID) {
        m_image = RemoteInterface::instance().discoveryImage();
        update();
        return;
    }

    if (!isComponentComplete())
        return;
    m_image = {};
    ImageStore::instance().requestImage(this, m_imageId, size(), m_type);
    update();
}

