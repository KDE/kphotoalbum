/* Copyright (C) 2003-2009 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef ABSTRACTCATEGORYMODEL_H
#define ABSTRACTCATEGORYMODEL_H
#include <QAbstractItemModel>
#include <DB/ImageSearchInfo.h>
#include <DB/CategoryPtr.h>

namespace Browser
{

/**
 * \brief Base class for Category models
 *
 * See \ref Browser for a detailed description of how this fits in with the rest of the classes in this module
 *
 * This class implements what is common for \ref FlatCategoryModel and \ref TreeCategoryModel.
 */
class AbstractCategoryModel :public QAbstractItemModel
{
public:
    OVERRIDE Qt::ItemFlags flags ( const QModelIndex& index ) const;
    OVERRIDE QVariant data( const QModelIndex & index, int role) const;
    OVERRIDE QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

protected:
    AbstractCategoryModel( const DB::CategoryPtr& category, const DB::ImageSearchInfo& info );

    bool hasNoneEntry() const;
    QString text( const QString& name ) const;
    QPixmap icon( const QString& name ) const;
    virtual QString indexToName(const QModelIndex& ) const = 0;

    DB::CategoryPtr _category;
    DB::ImageSearchInfo _info;
    QMap<QString, uint> _images;
    QMap<QString, uint> _videos;

};

}


#endif /* ABSTRACTCATEGORYMODEL_H */

