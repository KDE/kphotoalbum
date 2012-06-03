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

#include "CategoryPage.h"
#include "TreeCategoryModel.h"
#include "FlatCategoryModel.h"
#include <klocale.h>
#include "OverviewPage.h"
#include "BrowserWidget.h"
#include "enums.h"
#include <DB/ImageDB.h>
#include "ImageViewPage.h"

Browser::CategoryPage::CategoryPage( const DB::CategoryPtr& category, const DB::ImageSearchInfo& info, BrowserWidget* browser )
    : BrowserPage( info, browser ), _category( category ), _model( 0 )
{
}

void Browser::CategoryPage::activate()
{
    delete _model;
    if ( _category->viewType() == DB::Category::TreeView || _category->viewType() == DB::Category::ThumbedTreeView )
        _model = new TreeCategoryModel( _category, searchInfo() );
    else
        _model = new FlatCategoryModel( _category, searchInfo() );

    browser()->setModel( _model );
}

Browser::BrowserPage* Browser::CategoryPage::activateChild( const QModelIndex& index )
{
    const QString name = _model->data( index, ItemNameRole ).value<QString>();
    DB::ImageSearchInfo info = searchInfo();

    info.addAnd( _category->name(), name );
    if (static_cast<uint>(DB::ImageDB::instance()->search(info).size()) <= Settings::SettingsData::instance()->autoShowThumbnailView()) {
        browser()->addAction( new Browser::OverviewPage( Breadcrumb(name), info, browser() ) );
        return new ImageViewPage( info, browser() );
    } else
        return new Browser::OverviewPage( Breadcrumb(name), info, browser() );
}

DB::CategoryPtr Browser::CategoryPage::category() const
{
    return _category;
}

DB::Category::ViewType Browser::CategoryPage::viewType() const
{
    return _category->viewType();
}

bool Browser::CategoryPage::isViewChangeable() const
{
    return true;
}

