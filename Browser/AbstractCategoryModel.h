/* Copyright (C) 2003-2019 The KPhotoAlbum Development Team

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
#include <DB/Category.h>
#include <DB/CategoryPtr.h>
#include <DB/ImageSearchInfo.h>

#include <QAbstractItemModel>

namespace Browser
{

/**
 * \brief Base class for Category models
 *
 * See \ref Browser for a detailed description of how this fits in with the rest of the classes in this module
 *
 * This class implements what is common for \ref FlatCategoryModel and \ref TreeCategoryModel.
 */
class AbstractCategoryModel : public QAbstractItemModel
{
public:
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

protected:
    AbstractCategoryModel(const DB::CategoryPtr &category, const DB::ImageSearchInfo &info);

    bool hasNoneEntry() const;
    QString text(const QString &name) const;
    QPixmap icon(const QString &name) const;
    virtual QString indexToName(const QModelIndex &) const = 0;

    DB::CategoryPtr m_category;
    DB::ImageSearchInfo m_info;
    QMap<QString, DB::CountWithRange> m_images;
    QMap<QString, DB::CountWithRange> m_videos;
};

}

#endif /* ABSTRACTCATEGORYMODEL_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
