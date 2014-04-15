#include "ImageStore.h"
#include "ThumbnailModel.h"

namespace RemoteControl {

ThumbnailModel::ThumbnailModel(QObject *parent) :
    QAbstractListModel(parent)
{
}

int ThumbnailModel::rowCount(const QModelIndex&) const
{
    return m_images.count();
}

QVariant ThumbnailModel::data(const QModelIndex& index, int role) const
{
    if (role == ImageIdRole)
        return m_images[index.row()];
    return {};
}

RoleMap ThumbnailModel::roleNames() const
{
    return { {ImageIdRole, "imageId"} };
}

void ThumbnailModel::setImages(const QList<int>& images)
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
