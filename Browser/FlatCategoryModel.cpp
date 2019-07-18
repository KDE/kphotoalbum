/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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

#include "FlatCategoryModel.h"
#include <DB/ImageDB.h>
#include <KLocalizedString>

Browser::FlatCategoryModel::FlatCategoryModel(const DB::CategoryPtr &category, const DB::ImageSearchInfo &info)
    : AbstractCategoryModel(category, info)
{
    if (hasNoneEntry())
        m_items.append(DB::ImageDB::NONE());

    QStringList items = m_category->itemsInclCategories();
    items.sort();

    Q_FOREACH (const QString &name, items) {
        const int imageCount = m_images.contains(name) ? m_images[name].count : 0;
        const int videoCount = m_videos.contains(name) ? m_videos[name].count : 0;

        if (imageCount + videoCount > 0)
            m_items.append(name);
    }
}

int Browser::FlatCategoryModel::rowCount(const QModelIndex &index) const
{
    if (!index.isValid())
        return m_items.count();
    else
        return 0;
}

int Browser::FlatCategoryModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QModelIndex Browser::FlatCategoryModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row >= 0 && row < rowCount(parent) && column >= 0 && column < columnCount(parent))
        return createIndex(row, column);
    else
        return QModelIndex();
}

QModelIndex Browser::FlatCategoryModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

QString Browser::FlatCategoryModel::indexToName(const QModelIndex &index) const
{
    return m_items[index.row()];
}

// vi:expandtab:tabstop=4 shiftwidth=4:
