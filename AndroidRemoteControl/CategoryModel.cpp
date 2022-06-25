/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CategoryModel.h"
#include "ScreenInfo.h"
using namespace RemoteControl;

CategoryModel::CategoryModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int CategoryModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_categories.count();
}

QVariant CategoryModel::data(const QModelIndex &index, int role) const
{
    const Category &item = m_categories[index.row()];
    if (role == NameRole)
        return item.name;
    else if (role == IconRole)
        return item.icon;
    else if (role == EnabledRole)
        return item.enabled;
    else if (role == TypeRole)
        return item.viewType;
    return {};
}

RoleMap RemoteControl::CategoryModel::roleNames() const
{
    return { { NameRole, "name" }, { IconRole, "icon" }, { EnabledRole, "enabled" }, { TypeRole, "type" } };
}

void CategoryModel::setCategories(const QList<Category> &categories)
{
    beginResetModel();
    m_categories = categories;
    endResetModel();
    emit hasDataChanged();
}

bool CategoryModel::hasData() const
{
    return rowCount(QModelIndex()) != 0;
}

#include "moc_CategoryModel.cpp"
