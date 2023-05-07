// SPDX-FileCopyrightText: 2003-2013 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2013-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef CATEGORYPAGE_H
#define CATEGORYPAGE_H
#include "BrowserPage.h"

#include <DB/Category.h>
#include <DB/CategoryPtr.h>
#include <DB/search/ImageSearchInfo.h>

class QAbstractItemModel;
class FlatCategoryModel;
class BrowserWidget;

namespace Browser
{

/**
 * \brief The Browser page for categories.
 *
 * See \ref Browser for a detailed description of how this fits in with the rest of the classes in this module
 *
 */
class CategoryPage : public BrowserPage
{
public:
    CategoryPage(const DB::CategoryPtr &category, const DB::ImageSearchInfo &info, BrowserWidget *browser);
    void activate() override;
    BrowserPage *activateChild(const QModelIndex &) override;
    DB::Category::ViewType viewType() const override;
    bool isViewChangeable() const override;

    DB::CategoryPtr category() const;

private:
    const DB::CategoryPtr m_category;
    QAbstractItemModel *m_model;
};

}

#endif /* CATEGORYPAGE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
