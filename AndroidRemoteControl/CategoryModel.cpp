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
