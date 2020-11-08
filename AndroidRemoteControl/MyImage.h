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

#ifndef MYIMAGE_H
#define MYIMAGE_H

#include <QImage>
#include <QQuickPaintedItem>

namespace RemoteControl
{

class MyImage : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QImage image READ image WRITE setImage NOTIFY imageChanged)
    Q_PROPERTY(int imageWidth READ imageWidth NOTIFY imageWidthChanged)
    Q_PROPERTY(int imageHeight READ imageHeight NOTIFY imageHeightChanged)

public:
    explicit MyImage(QQuickItem *parent = 0);
    void paint(QPainter *painter) override;
    QImage image() const;

    int imageWidth() const;
    int imageHeight() const;

public slots:
    void setImage(const QImage &image);

signals:
    void imageChanged();

    void imageWidthChanged();
    void imageHeightChanged();

private:
    QImage m_image;
};

}
#endif // MYIMAGE_H
