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
#include "RemoteInterface.h"
#include "ScreenInfo.h"
#include "Settings.h"
#include <QPainter>

using namespace RemoteControl;

RemoteImage::RemoteImage(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
}

void RemoteImage::paint(QPainter *painter)
{
    painter->drawImage(0, 0, m_image);
}

int RemoteImage::imageId() const
{
    return m_imageId;
}

QSize RemoteImage::size() const
{
    if (m_type == ViewType::Images)
        return ScreenInfo::instance().viewSize();
    else
        return QSize(width(), height());
}

void RemoteImage::setLabel(const QString &label)
{
    if (label != m_label) {
        m_label = label;
        emit labelChanged();
    }
}

void RemoteImage::setImage(const QImage &image)
{
    m_image = image;
    setWidth(image.width());
    setHeight(image.height());
    update();
}

void RemoteImage::setImageId(int imageId)
{
    if (m_imageId != imageId) {
        m_imageId = imageId;
        emit imageIdChanged();
    }
}

void RemoteImage::loadFullSize()
{
    if (!m_hasFullSizedImage) {
        ImageStore::instance().requestImage(this, m_imageId, QSize(), m_type);
        m_hasFullSizedImage = true;
    }
}

void RemoteImage::componentComplete()
{
    QQuickPaintedItem::componentComplete();
    requestImage();
    if (m_type != ViewType::Images)
        connect(this, &RemoteImage::widthChanged, this, &RemoteImage::requestImage, Qt::QueuedConnection);
}

void RemoteImage::requestImage()
{
    if (!isComponentComplete())
        return;
    if (m_image.size() == size() && m_type != ViewType::Images)
        return;

    if (m_imageId == DISCOVERYID)
        m_image = RemoteInterface::instance().discoveryImage().scaled(size(), Qt::KeepAspectRatio);
    else {
        m_image = {};
        ImageStore::instance().requestImage(this, m_imageId, size(), m_type);
    }
    update();
}
