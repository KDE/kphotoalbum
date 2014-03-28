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
    painter->drawImage(0,0, RemoteInterface::instance().image(m_fileName));
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
        setSize(RemoteInterface::instance().image(fileName).size());
    }
}

void RemoteImage::updateImage(const QString& fileName)
{
    if (fileName == m_fileName) {
        setSize(RemoteInterface::instance().image(fileName).size());
        update();
    }
}
