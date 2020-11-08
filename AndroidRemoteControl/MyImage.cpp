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
