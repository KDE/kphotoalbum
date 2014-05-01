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

#ifndef REMOTEIMAGE_H
#define REMOTEIMAGE_H

#include <QQuickPaintedItem>
#include <QImage>
#include "Types.h"

namespace RemoteControl {

class Size :public QObject
{
    Q_OBJECT
    Q_PROPERTY(int width MEMBER m_width NOTIFY widthChanged)
    Q_PROPERTY(int height MEMBER m_height NOTIFY heightChanged)

public:
    void setSize(const QSize& size);

signals:
    void widthChanged();
    void heightChanged();

private:
    int m_width = 2000;
    int m_height = 2000;
};

class RemoteImage : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(int imageId READ imageId WRITE setImageId NOTIFY imageIdChanged)
    Q_PROPERTY(RemoteControl::Types::ViewType type MEMBER m_type NOTIFY typeChanged)
    Q_PROPERTY(QString label MEMBER m_label NOTIFY labelChanged)
    Q_PROPERTY(RemoteControl::Size* sourceSize MEMBER m_sourceSize NOTIFY sourceSizeChanged)

public:
    explicit RemoteImage(QQuickItem *parent = 0);
    void paint(QPainter *painter) override;
    int imageId() const;
    QSize size() const;
    void setLabel(const QString& label);
    void setImage(const QImage& image);

public slots:
    void setImageId(int imageId);

protected:
    void componentComplete();

private slots:
    void requestImage();

signals:
    void imageIdChanged();
    void typeChanged();
    void labelChanged();
    void sourceSizeChanged();

private:
    int m_imageId;
    ViewType m_type;
    QString m_label;
    QImage m_image;
    Size* m_sourceSize;
};

}

Q_DECLARE_METATYPE(RemoteControl::Size*);

#endif // REMOTEIMAGE_H
