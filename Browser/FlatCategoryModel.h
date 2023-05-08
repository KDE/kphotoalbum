// SPDX-FileCopyrightText: 2003-2014 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2013-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FLATCATEGORYMODEL_H
#define FLATCATEGORYMODEL_H
#include "AbstractCategoryModel.h"

#include <DB/Category.h>
#include <DB/search/ImageSearchInfo.h>

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
