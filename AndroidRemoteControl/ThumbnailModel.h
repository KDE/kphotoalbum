/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
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
    enum { ImageIdRole,
           IsVideoRole };
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
