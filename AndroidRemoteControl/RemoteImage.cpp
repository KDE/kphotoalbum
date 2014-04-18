#include "RemoteImage.h"
#include "ImageStore.h"
#include <QPainter>
#include "Settings.h"

using namespace RemoteControl;

RemoteImage::RemoteImage(QQuickItem *parent) :
    QQuickPaintedItem(parent)
{
}

void RemoteImage::paint(QPainter* painter)
{
    const QImage image = ImageStore::instance().image(this, m_imageId, size(), m_type);
    painter->drawImage((width() - image.width()) / 2, height() - image.height(), image);
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

void RemoteImage::setImageId(int imageId)
{
    if (m_imageId != imageId) {
        m_imageId = imageId;
        setLabel(ImageStore::instance().label(m_imageId));
        emit imageIdChanged();
    }
}

