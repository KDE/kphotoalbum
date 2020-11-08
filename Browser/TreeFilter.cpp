/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "TreeFilter.h"

Browser::TreeFilter::TreeFilter(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool Browser::TreeFilter::filterAcceptsRow(int row, const QModelIndex &parent) const
{
    bool match = false;
    bool openAllChildren = false;

    // If parent is open then child should be included.
    if (m_matchedMap[parent]) {
        match = true;
        openAllChildren = true;
    }

    // check if the item itself matches
    else if (QSortFilterProxyModel::filterAcceptsRow(row, parent)) {
        match = true;
        openAllChildren = true;
    } else {
        // Check if any children matches
        const QModelIndex myModelIndex = sourceModel()->index(row, 0, parent);
        const int childCount = sourceModel()->rowCount(myModelIndex);
        for (int i = 0; i < childCount; ++i) {
            if (filterAcceptsRow(i, myModelIndex)) {
                match = true;
                break;
            }
        }
    }

    m_matchedMap[sourceModel()->index(row, 0, parent)] = openAllChildren;
    return match;
}

void Browser::TreeFilter::resetCache()
{
    m_matchedMap.clear();
}
// vi:expandtab:tabstop=4 shiftwidth=4:
