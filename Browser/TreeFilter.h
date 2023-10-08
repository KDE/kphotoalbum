// SPDX-FileCopyrightText: 2003 - 2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef TREEFILTER_H
#define TREEFILTER_H
#include <QSortFilterProxyModel>
#include <memory>

class QCollator;

namespace Browser
{
/**
 * \brief Filter proxy that keeps parent branches if child branches matches
 * Additionally, it supports changing the sort order from QString comparison to localized numeric sort (aka natural sort order).
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

    /**
     * @brief setNaturalSortOrder sets the sort order to be natural or not.
     * "Natural" sort order sorts according to the current locale and properly sorts numeric values, so that e.g. 2 sorts before 10.
     *
     * Note: Natural sort order is more expensive than string comparison, so even if it defaults to being enabled, it needs to be user-configurable for people with large databases.
     * @param naturalSortOrder
     */
    void setNaturalSortOrder(bool naturalSortOrder);

    /**
     * @brief naturalSortOrder
     * @return \c true, if natural sort order is enabled, \c false otherwise.
     */
    bool naturalSortOrder() const;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    /**
     * @brief lessThan overrides QSortFilterProxyModel::lessThan to enable "natural" sorting of numeric values.
     * @see setNaturalSort(bool)
     */
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

    mutable QMap<QModelIndex, bool> m_matchedMap;

private:
    bool m_naturalSortOrder = true;
    std::unique_ptr<QCollator> m_naturalCollator;
};
}

#endif /* TREEFILTER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
