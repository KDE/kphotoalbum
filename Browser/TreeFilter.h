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
class TreeFilter :public QSortFilterProxyModel
{
public:
    explicit TreeFilter( QObject* parent = nullptr );
    void resetCache();

protected:
    bool filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const override;

    mutable QMap<QModelIndex,bool> m_matchedMap;
};
}

#endif /* TREEFILTER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
