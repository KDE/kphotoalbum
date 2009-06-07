#include "CategoryPage.h"
#include "TreeCategoryModel.h"
#include "FlatCategoryModel.h"
#include <DB/MemberMap.h>
#include <klocale.h>
#include <DB/ImageDB.h>
#include <QDebug>
#include <Settings/SettingsData.h>
#include "OverviewPage.h"
#include "BrowserWidget.h"
#include <DB/CategoryItem.h>
#include <KIcon>
#include "enums.h"

class CategoryItem :public QStandardItem
{

};

Browser::CategoryPage::CategoryPage( const DB::CategoryPtr& category, const DB::ImageSearchInfo& info, BrowserWidget* browser )
    : BrowserPage( info, browser ), _category( category ), _model( 0 )
{
}

void Browser::CategoryPage::activate()
{
    delete _model;
    if ( _category->viewType() == DB::Category::ListView || _category->viewType() == DB::Category::ThumbedListView )
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

