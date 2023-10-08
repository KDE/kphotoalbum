// SPDX-FileCopyrightText: 2003 - 2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "TreeFilter.h"

#include <QCollator>

Browser::TreeFilter::TreeFilter(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_naturalCollator(std::make_unique<QCollator>())
{
    m_naturalCollator->setNumericMode(true);
    m_naturalCollator->setIgnorePunctuation(true); // only works if Qt is set up to use ICU
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

bool Browser::TreeFilter::naturalSortOrder() const
{
    return m_naturalSortOrder;
}

void Browser::TreeFilter::setNaturalSortOrder(bool naturalSortOrder)
{
    m_naturalSortOrder = naturalSortOrder;
}

bool Browser::TreeFilter::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    if (m_naturalSortOrder) {
        // numeric sort
        const QString &string_left = source_left.data(sortRole()).toString();
        const QString &string_right = source_right.data(sortRole()).toString();
        return m_naturalCollator->compare(string_left, string_right) < 0;
    } else {
        return QSortFilterProxyModel::lessThan(source_left, source_right);
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:
