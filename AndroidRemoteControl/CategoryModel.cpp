#include "CategoryModel.h"

using namespace RemoteControl;

CategoryModel::CategoryModel(QObject *parent) :
    QAbstractListModel(parent)
{
}

int CategoryModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_categories.count();
}

QVariant CategoryModel::data(const QModelIndex& index, int role) const
{
    const Category& item = m_categories[index.row()];
    if (role == NameRole)
        return item.name;
    else if (role == TextRole)
        return item.text;
    else if (role == IconRole)
        return item.icon;
    else if (role == EnabledRole)
        return item.enabled;
    return {};
}

QHash<int, QByteArray> RemoteControl::CategoryModel::roleNames() const
{
    QHash<int, QByteArray> result;
    result.insert(NameRole, "name");
    result.insert(TextRole, "text");
    result.insert(IconRole, "icon");
    result.insert(EnabledRole, "enabled");
    return result;
}

void CategoryModel::setCategories(const QList<Category>& categories)
{
    beginResetModel();
    m_categories = categories;
    endResetModel();
}
