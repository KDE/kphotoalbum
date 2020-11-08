/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TREEFILTER_H
#define TREEFILTER_H
#include <QSortFilterProxyModel>

namespace Browser
{
/**
 * \brief Filter proxy that keeps parent branches if child branches matches
 *
 * See \ref Browser for a detailed description of how this fits in with the rest of the classes in this module
 *
 * The QSortFilterProxyModel has one drawback that makes it inappropriate to
 * use for filtering in the browser, namely that it hides an item, if the item
 * doesn't match the filter criteria, even if child items actually do match
 * the filter criteria. This class overcomes this shortcoming.
 */
class TreeFilter : public QSortFilterProxyModel
{
public:
    explicit TreeFilter(QObject *parent = nullptr);
    void resetCache();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    mutable QMap<QModelIndex, bool> m_matchedMap;
};
}

#endif /* TREEFILTER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
