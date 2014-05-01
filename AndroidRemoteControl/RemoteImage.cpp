/* Copyright (C) 2014 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "RemoteImage.h"
#include "ImageStore.h"
#include <QPainter>
#include "RemoteInterface.h"
#include "Settings.h"

using namespace RemoteControl;

RemoteImage::RemoteImage(QQuickItem *parent) :
    QQuickPaintedItem(parent), m_sourceSize(new Size)
{
    //connect(this, &RemoteImage::widthChanged, this, &RemoteImage::requestImage);
    qRegisterMetaType<RemoteControl::Size*>("RemoteControl::Size*");
}

void RemoteImage::paint(QPainter* painter)
{
    qDebug("Painting %d x %d image is %d x %d", (int)width(),(int)height(),m_image.width(), m_image.height());
    painter->drawImage(0,0, m_image.scaled(size()));
    //    if (m_type == ViewType::Images)
//        qDebug("Oainting: width=%d imageWidth:%d", (int) width(), m_image.width());
//    const int x = (width() - m_image.width()) / 2;
//    int y = height() - m_image.height();
//    if (m_type == ViewType::Images)
//        y /= 2;
//    painter->drawImage(x, y, m_image);
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
    m_sourceSize->setSize(image.size());
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
    if (!isComponentComplete())
        return;
    if (m_imageId == DISCOVERYID)
        m_image = RemoteInterface::instance().discoveryImage().scaled(size(), Qt::KeepAspectRatio);
    else {
        m_image = {};
        if (m_type == ViewType::Images)
            ImageStore::instance().requestImage(this, m_imageId, QSize(), m_type);
        else
            ImageStore::instance().requestImage(this, m_imageId, size(), m_type);
    }
    update();
}





void Size::setSize(const QSize &size)
{
qDebug() << size;
    if (size != QSize(m_width,m_height)) {
        m_width = size.width();
        m_height = size.height();
        emit widthChanged();
        emit heightChanged();
    }
}
