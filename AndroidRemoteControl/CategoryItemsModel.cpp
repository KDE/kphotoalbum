#include "CategoryItemsModel.h"
#include "Settings.h"

namespace RemoteControl {

CategoryItemsModel::CategoryItemsModel(QObject *parent) :
    QAbstractListModel(parent)
{
    connect(&Settings::instance(), &Settings::categoryItemSizeChanged, this, &CategoryItemsModel::reset);
}

int CategoryItemsModel::rowCount(const QModelIndex& ) const
{
    return m_items.count();
}

QVariant CategoryItemsModel::data(const QModelIndex& index, int role) const
{
    if (role == TextRole)
        return m_items[index.row()].text;
    else if (role == IconRole) {
        int size = Settings::instance().categoryItemSize();
        return m_items[index.row()].icon.scaled(size, size,Qt::KeepAspectRatio);
    }
    return {};
}

RoleMap CategoryItemsModel::roleNames() const
{
    return { {TextRole, "text"}, {IconRole, "icon"} };
}

void CategoryItemsModel::setItems(const CategoryItemsList& items)
{
    beginResetModel();
    m_items = items;
    endResetModel();
}

void CategoryItemsModel::reset()
{
    beginResetModel();
    endResetModel();
}

} // namespace RemoteControl
