#include "MyImage.h"
#include <QPainter>

using namespace RemoteControl;

MyImage::MyImage(QQuickItem *parent) :
    QQuickPaintedItem(parent)
{
}

void MyImage::paint(QPainter* painter)
{
    painter->drawImage(0,0, m_image);
}

QImage MyImage::image() const
{
    return m_image;
}

void MyImage::setImage(const QImage& image)
{
    if (m_image != image) {
        m_image = image;
        setSize(image.size());
        emit imageChanged();
    }
}
