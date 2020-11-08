/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "MyImage.h"
#include <QPainter>

using namespace RemoteControl;

MyImage::MyImage(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
}

void MyImage::paint(QPainter *painter)
{
    painter->drawImage(0, 0, m_image);
}

QImage MyImage::image() const
{
    return m_image;
}

int MyImage::imageWidth() const
{
    return m_image.width();
}

int MyImage::imageHeight() const
{
    return m_image.height();
}

void MyImage::setImage(const QImage &image)
{
    if (m_image != image) {
        m_image = image;
        emit imageChanged();
        emit imageWidthChanged();
        emit imageHeightChanged();
    }
}
