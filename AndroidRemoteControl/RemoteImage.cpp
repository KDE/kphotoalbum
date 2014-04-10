#include "RemoteImage.h"
#include "ImageStore.h"
#include <QPainter>
#include "Settings.h"

using namespace RemoteControl;

RemoteImage::RemoteImage(QQuickItem *parent) :
    QQuickPaintedItem(parent)
{
    connect(&ImageStore::instance(), &ImageStore::imageUpdated, this, &RemoteImage::updateImage);
}

void RemoteImage::paint(QPainter* painter)
{
    painter->drawImage(0,0, ImageStore::instance().image(m_fileName, size(), (ViewType) m_type)); // FIXME: Nuke the cast
}

QString RemoteImage::fileName() const
{
    return m_fileName;
}

QSize RemoteImage::size() const
{
    return QSize(width(),height());
}

void RemoteImage::setFileName(const QString& fileName)
{
    if (m_fileName != fileName) {
        m_fileName = fileName;
        emit fileNameChanged();
        if (m_type == (int) ViewType::Thumbnails || m_type == (int) ViewType::CategoryItems) { // FIXME, should be two different sizes
            const int size = Settings::instance().thumbnailSize();
            setSize(QSize(size,size));
        }
    }
}

void RemoteImage::updateImage(const QString& fileName, ViewType type)
{
    if (fileName == m_fileName && (int) type == m_type) // FIXME no cast
        update();
}
