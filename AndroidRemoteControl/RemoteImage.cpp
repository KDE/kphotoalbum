// SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

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
        Q_EMIT labelChanged();
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
        Q_EMIT imageIdChanged();
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

#include "moc_RemoteImage.cpp"
