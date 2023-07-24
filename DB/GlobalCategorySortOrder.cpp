// SPDX-FileCopyrightText: 2003-2023 The KPhotoAlbum Development Team
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "GlobalCategorySortOrder.h"
#include "CategoryCollection.h"
#include "ImageDB.h"
#include <QDebug>
#include <algorithm>
#include <set>

namespace DB
{

void GlobalCategorySortOrder::pushToFront(const QString &category, const QString &value)
{
    const Item item { category, value };
    m_sortOrder.removeAll(item);
    m_sortOrder.push_front(item);
}

QList<GlobalCategorySortOrder::Item> GlobalCategorySortOrder::modifiedSortOrder()
{
    return m_sortOrder;
}

QList<GlobalCategorySortOrder::Item> GlobalCategorySortOrder::completeSortOrder()
{
    QList<Item> result;
    result.append(m_sortOrder);

    // This is the set of all those categories already kept at the front.
    // We have it as a set to allow fast lookup when going through all categories
    // and appending them (as those found here should not be appended)
    QSet<Item> currentSet(m_sortOrder.cbegin(), m_sortOrder.cend());

    // To make it possible to remove entries no longer present in the categories
    // (either because they were removed or renamed, or gone some other magic way),
    // we initial copy all categories from m_sortOrder, and then remove them as we see them
    // while populating from each category
    QSet<Item> unknownSet = currentSet;

    const auto categories = DB::ImageDB::instance()->categoryCollection()->categories();
    for (const auto &category : categories) {
        if (!category->isSpecialCategory() || category->type() == DB::Category::TokensCategory) {
            auto items = category->items();

            for (const auto &categoryItem : qAsConst(items)) {
                const Item item { category->name(), categoryItem };
                unknownSet.remove(item);
                if (!currentSet.contains(item))
                    result.append(item);
            }
        }
    }

    for (const auto &item : unknownSet) {
        result.removeAll(item);
    }

    return result;
}

bool operator==(const GlobalCategorySortOrder::Item &x, const GlobalCategorySortOrder::Item &y)
{
    return x.category == y.category && x.item == y.item;
}

size_t qHash(const GlobalCategorySortOrder::Item &item)
{
    return qHash(item.category) + qHash(item.item);
}

} // namespace DB
