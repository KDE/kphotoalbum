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

#ifndef FLATCATEGORYMODEL_H
#define FLATCATEGORYMODEL_H
#include "AbstractCategoryModel.h"
#include <DB/Category.h>
#include <DB/ImageSearchInfo.h>

namespace RemoteControl
{
class RemoteInterface;
}
namespace Browser
{

/**
 * \brief Implements a flat model for categories - ie. a model where all catergories, including subcategories, are shown.
 *
 * See \ref Browser for a detailed description of how this fits in with the rest of the classes in this module
 */
class FlatCategoryModel : public AbstractCategoryModel
{
public:
    FlatCategoryModel(const DB::CategoryPtr &category, const DB::ImageSearchInfo &info);
    int rowCount(const QModelIndex &index) const override;

    int columnCount(const QModelIndex &) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    QString indexToName(const QModelIndex &) const override;

private:
    friend class RemoteControl::RemoteInterface;
    QStringList m_items;
};

}

#endif /* FLATCATEGORYMODEL_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
