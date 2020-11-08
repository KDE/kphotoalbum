/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
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
