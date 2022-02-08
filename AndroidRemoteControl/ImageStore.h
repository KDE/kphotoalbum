// SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef REMOTECONTROL_IMAGESTORE_H
#define REMOTECONTROL_IMAGESTORE_H

#include "../RemoteControl/Types.h"
#include <QDate>
#include <QHash>
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
    void setImageDates(const QHash<ImageId, QDate> &imageDates);
    QDate date(ImageId id) const;

private Q_SLOTS:
    void reset();
    void clientDeleted();

private:
    explicit ImageStore();

    using RequestType = QPair<ImageId, ViewType>;
    QMap<RequestType, RemoteImage *> m_requestMap;
    QMap<RemoteImage *, RequestType> m_reverseRequestMap;
    QHash<ImageId, QDate> m_imageDates; // Will only be accessed on GUI thread
    QMutex m_mutex;
};

} // namespace RemoteControl

#endif // REMOTECONTROL_IMAGESTORE_H
