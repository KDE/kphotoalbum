// SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef REMOTEIMAGE_H
#define REMOTEIMAGE_H

#include "../RemoteControl/Types.h"
#include <QImage>
#include <QQuickPaintedItem>

namespace RemoteControl
{

class RemoteImage : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(int imageId READ imageId WRITE setImageId NOTIFY imageIdChanged)
    Q_PROPERTY(RemoteControl::Types::ViewType type MEMBER m_type NOTIFY typeChanged)
    Q_PROPERTY(QString label MEMBER m_label NOTIFY labelChanged)

public:
    explicit RemoteImage(QQuickItem *parent = 0);
    void paint(QPainter *painter) override;
    int imageId() const;
    QSize size() const;
    void setLabel(const QString &label);
    void setImage(const QImage &image);

public Q_SLOTS:
    void setImageId(int imageId);
    void loadFullSize();

protected:
    void componentComplete() override;

private Q_SLOTS:
    void requestImage();

Q_SIGNALS:
    void imageIdChanged();
    void typeChanged();
    void labelChanged();
    void sourceSizeChanged();

private:
    int m_imageId;
    ViewType m_type;
    QString m_label;
    QImage m_image;
    bool m_hasFullSizedImage = false;
};

}

#endif // REMOTEIMAGE_H
