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

#ifndef REMOTECONTROL_THUMBNAILMODEL_H
#define REMOTECONTROL_THUMBNAILMODEL_H

#include <QAbstractListModel>

namespace RemoteControl
{

using RoleMap = QHash<int, QByteArray>;
class ThumbnailModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ThumbnailModel(QObject *parent = 0);
    enum { ImageIdRole };
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    RoleMap roleNames() const override;
    virtual void setImages(const QList<int> &image);
    int indexOf(int imageId);

protected:
    QList<int> m_images;
};

} // namespace RemoteControl

Q_DECLARE_METATYPE(RemoteControl::ThumbnailModel *);

#endif // REMOTECONTROL_THUMBNAILMODEL_H
