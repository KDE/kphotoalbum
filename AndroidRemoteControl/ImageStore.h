// SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef REMOTECONTROL_IMAGESTORE_H
#define REMOTECONTROL_IMAGESTORE_H

#include "../RemoteControl/Types.h"
#include <QImage>
#include <QMap>
#include <QMutex>
#include <QObject>

namespace RemoteControl
{
class RemoteImage;

class ImageStore : public QObject
{
    Q_OBJECT

public:
    static ImageStore &instance();
    void updateImage(ImageId imageId, const QImage &requestImage, const QString &label, ViewType type);
    void requestImage(RemoteImage *client, ImageId imageId, const QSize &size, ViewType type);

private Q_SLOTS:
    void reset();
    void clientDeleted();

private:
    explicit ImageStore();

    using RequestType = QPair<ImageId, ViewType>;
    QMap<RequestType, RemoteImage *> m_requestMap;
    QMap<RemoteImage *, RequestType> m_reverseRequestMap;
    QMutex m_mutex;
};

} // namespace RemoteControl

#endif // REMOTECONTROL_IMAGESTORE_H
