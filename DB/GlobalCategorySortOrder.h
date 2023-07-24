// SPDX-FileCopyrightText: 2003-2023 The KPhotoAlbum Development Team
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "CategoryPtr.h"
#include <QList>

namespace DB
{
class FileReader;

/**
   Just as each category has a most-recently-used sorting,
   that makes it possible to see the most likely hit in the List boxes in the annotation dialog,
   we also keep a global most-recent-used list across all categories, so that you can
   get a sorting like People/Jesper, Places/Las Vegas, People Jim
   This is what this class encapsulate.

   When an item has been used for tagging, it is pushed to the front of the list using
   \ref pushToFront.

   To get the global sort order for use for completion, call \ref completeSortOrder.

   On her other hand, to save the sort order, only those ever used should be retrieved,
   which is possible using \ref modifiedSortOrder
 */
class GlobalCategorySortOrder
{
public:
    void pushToFront(const QString &category, const QString &value);

    struct Item {
        QString category;
        QString item;
        friend bool operator==(const Item &x, const Item &y);
        friend size_t qHash(const Item &item);
    };

    QList<Item> completeSortOrder();
    QList<Item> modifiedSortOrder();

private:
    friend class DB::FileReader;
    QList<Item> m_sortOrder;
};

} // namespace DB
