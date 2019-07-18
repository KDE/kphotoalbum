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
