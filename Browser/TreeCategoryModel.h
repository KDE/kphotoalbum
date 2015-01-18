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

#ifndef TREECATEGORYMODEL_H
#define TREECATEGORYMODEL_H
#include "AbstractCategoryModel.h"
namespace DB { class CategoryItem; }

namespace Browser
{

/**
 * \brief A QAbstractItemModel subclass that represent the items of a given category as a tree
 *
 * See \ref Browser for a detailed description of how this fits in with the rest of the classes in this module
 *
 * This class implements the QAbstractItemModel interface, which is
 * actually what most of the methods is about. The constructor queries
 * the category information from the back end, and builds an internal
 * data structure representing the tree. It does build its own data structure for two reasons:
 * \li The \ref DB::CategoryItem's do not have an easy way to go from child
 * to parent, something that was needed by the \ref parent method. It was
 * considered too risky to add that to the \ref DB::CategoryItem
 * data structure at the time this was implemented.
 * \li By building its own data structure it can ensure that the data is
 * not changing behind the scene, something that might have happened if
 * this class was constructed, categories was added or removed, and the
 * class was asked information abouts its data.
 */
class TreeCategoryModel : public AbstractCategoryModel
{
public:
    TreeCategoryModel( const DB::CategoryPtr& category, const DB::ImageSearchInfo& info );
    ~TreeCategoryModel();

    int rowCount( const QModelIndex& ) const override;
    int columnCount( const QModelIndex& ) const override;
    QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const override;
    QModelIndex parent ( const QModelIndex & index ) const override;

    QString indexToName(const QModelIndex& ) const override;

private:
    struct Data;
    bool createData( DB::CategoryItem* parentCategoryItem, Data* parent );
    Data* indexToData( const QModelIndex& index ) const;

private:
    Data* m_data;
};

}

#endif /* TREECATEGORYMODEL_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
