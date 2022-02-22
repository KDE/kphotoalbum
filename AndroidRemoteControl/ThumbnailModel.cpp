/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ThumbnailModel.h"
#include "VideoStore.h"
#include <QDebug>

namespace RemoteControl
{

ThumbnailModel::ThumbnailModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int ThumbnailModel::rowCount(const QModelIndex &) const
{
    return m_images.count();
}

QVariant ThumbnailModel::data(const QModelIndex &index, int role) const
{
    auto id = [&] { return m_images[index.row()]; };
    if (role == ImageIdRole)
        return id();
    else if (role == IsVideoRole)
        return VideoStore::instance().isVideo(id());
    Q_UNREACHABLE();
    return {};
}

RoleMap ThumbnailModel::roleNames() const
{
    return { { ImageIdRole, "imageId" }, { IsVideoRole, "isVideo" } };
}

void ThumbnailModel::setImages(const QList<int> &images)
{
    beginResetModel();
    m_images = images;
    endResetModel();
}

int ThumbnailModel::indexOf(int imageId)
{
    return m_images.indexOf(imageId);
}

} // namespace RemoteControl

#include "moc_ThumbnailModel.cpp"
