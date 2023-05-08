// SPDX-FileCopyrightText: 2009 Jan Kundrát <jkt@flaska.net>
// SPDX-FileCopyrightText: 2009-2013 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2013-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef ABSTRACTCATEGORYMODEL_H
#define ABSTRACTCATEGORYMODEL_H
#include <DB/Category.h>
#include <DB/CategoryPtr.h>
#include <DB/search/ImageSearchInfo.h>

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
