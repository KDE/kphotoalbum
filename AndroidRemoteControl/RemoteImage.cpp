#include "RemoteImage.h"
#include "RemoteInterface.h"
#include <QPainter>

using namespace RemoteControl;

RemoteImage::RemoteImage(QQuickItem *parent) :
    QQuickPaintedItem(parent)
{
    connect(&RemoteInterface::instance(), &RemoteInterface::imageUpdated, this, &RemoteImage::updateImage);
}

void RemoteImage::paint(QPainter* painter)
{
    painter->drawImage(0,0, RemoteInterface::instance().image(m_index));
}

int RemoteImage::index() const
{
    return m_index;
}

void RemoteImage::setIndex(int index)
{
    if (m_index != index) {
        m_index = index;
        emit indexChanged();
        setSize(RemoteInterface::instance().image(index).size());
    }
}

void RemoteImage::updateImage(int index)
{
    if (index == m_index) {
        setSize(RemoteInterface::instance().image(index).size());
        update();
    }
}
