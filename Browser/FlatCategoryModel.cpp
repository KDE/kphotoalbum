// SPDX-FileCopyrightText: 2003-2020 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2024 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "FlatCategoryModel.h"

#include <DB/ImageDB.h>

#include <KLocalizedString>

#include <utility>

Browser::FlatCategoryModel::FlatCategoryModel(const DB::CategoryPtr &category, const DB::ImageSearchInfo &info)
    : AbstractCategoryModel(category, info)
{
    if (hasNoneEntry())
        m_items.append(DB::ImageDB::NONE());

    QStringList items = m_category->itemsInclCategories();
    items.sort();

    for (const QString &name : std::as_const(items)) {
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
