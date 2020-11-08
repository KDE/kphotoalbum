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

#ifndef REMOTECONTROL_IMAGESTORE_H
#define REMOTECONTROL_IMAGESTORE_H

#include "Types.h"
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

private slots:
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
