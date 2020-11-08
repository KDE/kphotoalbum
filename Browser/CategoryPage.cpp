/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CategoryPage.h"

#include "BrowserWidget.h"
#include "FlatCategoryModel.h"
#include "ImageViewPage.h"
#include "OverviewPage.h"
#include "TreeCategoryModel.h"
#include "enums.h"

#include <DB/ImageDB.h>

#include <KLocalizedString>

Browser::CategoryPage::CategoryPage(const DB::CategoryPtr &category, const DB::ImageSearchInfo &info, BrowserWidget *browser)
    : BrowserPage(info, browser)
    , m_category(category)
    , m_model(nullptr)
{
}

void Browser::CategoryPage::activate()
{
    delete m_model;
    if (m_category->viewType() == DB::Category::TreeView || m_category->viewType() == DB::Category::ThumbedTreeView)
        m_model = new TreeCategoryModel(m_category, searchInfo());
    else
        m_model = new FlatCategoryModel(m_category, searchInfo());

    browser()->setModel(m_model);
}

Browser::BrowserPage *Browser::CategoryPage::activateChild(const QModelIndex &index)
{
    const QString name = m_model->data(index, ItemNameRole).value<QString>();
    DB::ImageSearchInfo info = searchInfo();

    info.addAnd(m_category->name(), name);
    if (DB::ImageDB::instance()->search(info).size() <= Settings::SettingsData::instance()->autoShowThumbnailView()) {
        browser()->addAction(new Browser::OverviewPage(Breadcrumb(name), info, browser()));
        return new ImageViewPage(info, browser());
    } else
        return new Browser::OverviewPage(Breadcrumb(name), info, browser());
}

DB::CategoryPtr Browser::CategoryPage::category() const
{
    return m_category;
}

DB::Category::ViewType Browser::CategoryPage::viewType() const
{
    return m_category->viewType();
}

bool Browser::CategoryPage::isViewChangeable() const
{
    return true;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
