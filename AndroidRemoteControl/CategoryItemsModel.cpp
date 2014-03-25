#include "CategoryItemsModel.h"

namespace RemoteControl {

CategoryItemsModel::CategoryItemsModel(QObject *parent) :
    QAbstractListModel(parent)
{
}

int CategoryItemsModel::rowCount(const QModelIndex& ) const
{
    return m_items.count();
}

QVariant CategoryItemsModel::data(const QModelIndex& index, int role) const
{
    if (role == TextRole)
        return m_items[index.row()].text;
    else if (role == IconRole)
        return m_items[index.row()].icon;
    return {};
}

QHash<int, QByteArray> CategoryItemsModel::roleNames() const
{
    QHash<int, QByteArray> result;
    result.insert(TextRole, "text");
    result.insert(IconRole, "icon");
    return result;
}

void CategoryItemsModel::setItems(const CategoryItemsList& items)
{
    beginResetModel();
    m_items = items;
    endResetModel();
}

} // namespace RemoteControl
