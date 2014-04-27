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
    if (!isComponentComplete())
        return;
    if (m_imageId == DISCOVERYID)
        m_image = RemoteInterface::instance().discoveryImage().scaled(size(), Qt::KeepAspectRatio);
    else {
        m_image = {};
        ImageStore::instance().requestImage(this, m_imageId, size(), m_type);
    }
    update();
}

