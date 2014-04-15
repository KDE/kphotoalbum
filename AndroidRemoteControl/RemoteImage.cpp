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
    painter->drawImage(0,0, ImageStore::instance().image(this, m_imageId, size(), (ViewType) m_type)); // FIXME: Nuke the cast
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
        emit imageIdChanged();
        if (m_type == (int) ViewType::Thumbnails || m_type == (int) ViewType::CategoryItems) { // FIXME, should be two different sizes
            const int size = Settings::instance().thumbnailSize();
            setSize(QSize(size,size));
        }
    }
}

