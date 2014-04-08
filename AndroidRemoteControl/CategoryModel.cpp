#include "CategoryModel.h"
#include "ScreenInfo.h"
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
    else if (role == IconRole) {
        int size = ScreenInfo::instance().overviewIconSize();
        return item.icon.scaled(size,size,Qt::KeepAspectRatio);
    }
    else if (role == EnabledRole)
        return item.enabled;
    return {};
}

RoleMap RemoteControl::CategoryModel::roleNames() const
{
    return { {NameRole, "name"}, {TextRole, "text"}, {IconRole, "icon"}, {EnabledRole, "enabled"} };
}

void CategoryModel::setCategories(const QList<Category>& categories)
{
    beginResetModel();
    m_categories = categories;
    endResetModel();
}
