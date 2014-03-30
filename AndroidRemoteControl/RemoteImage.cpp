#include "RemoteImage.h"
#include "ImageStore.h"
#include <QPainter>

using namespace RemoteControl;

RemoteImage::RemoteImage(QQuickItem *parent) :
    QQuickPaintedItem(parent)
{
    connect(&ImageStore::instance(), &ImageStore::imageUpdated, this, &RemoteImage::updateImage);
}

void RemoteImage::paint(QPainter* painter)
{
    painter->drawImage(0,0, ImageStore::instance().image(m_fileName));
}

QString RemoteImage::fileName() const
{
    return m_fileName;
}

void RemoteImage::setFileName(const QString& fileName)
{
    if (m_fileName != fileName) {
        m_fileName = fileName;
        emit fileNameChanged();
        setSize(ImageStore::instance().image(fileName).size());
    }
}

void RemoteImage::updateImage(const QString& fileName)
{
    if (fileName == m_fileName) {
        setSize(ImageStore::instance().image(fileName).size());
        update();
    }
}
