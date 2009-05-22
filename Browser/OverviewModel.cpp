#include "OverviewModel.h"
#include "CategoryModel.h"
#include "BrowserWidget.h"
#include <MainWindow/Window.h>
#include <QDebug>
#include <klocale.h>
#include <DB/ImageDB.h>
#include <kicon.h>

Browser::OverviewModel::OverviewModel( const DB::ImageSearchInfo& info, BrowserWidget* browser )
    : BrowserAction(browser), _info(info)
{
}

int Browser::OverviewModel::rowCount( const QModelIndex& /*parent*/ ) const
{
    return categories().count() +
#ifdef HAVE_EXIV2
        1 +
#endif
        2; // Search info + Show Image
}

QVariant Browser::OverviewModel::data( const QModelIndex& index, int role) const
{
    const int row = index.row();
    if ( isCategoryIndex(row) )
        return categoryInfo( row, role );
    else if ( isExivIndex( row ) )
        return exivInfo( role );
    else if ( isSearchIndex( row ) )
        return searchInfo( role );
    else if ( isImageIndex( row ) )
        return imageInfo( role );
    return QVariant();
}

bool Browser::OverviewModel::isCategoryIndex( int row ) const
{
    return row < categories().count();
}

bool Browser::OverviewModel::isExivIndex( int row ) const
{
#ifdef HAVE_EXIV2
    return row == categories().count();
#else
    return false;
#endif
}

bool Browser::OverviewModel::isSearchIndex( int row ) const
{
    return rowCount()-2 == row;
}

bool Browser::OverviewModel::isImageIndex( int row ) const
{
    return rowCount()-1 == row;
}


QList<DB::CategoryPtr> Browser::OverviewModel::categories() const
{
    return DB::ImageDB::instance()->categoryCollection()->categories();
}

QVariant Browser::OverviewModel::categoryInfo( int row, int role ) const
{
    if ( role == Qt::DisplayRole )
        return categories()[row]->text();
    else if ( role == Qt::DecorationRole )
        return categories()[row]->icon(42);

    return QVariant();
}

QVariant Browser::OverviewModel::exivInfo( int role ) const
{
    if ( role == Qt::DisplayRole )
        return i18n("Exif Info");
    else if ( role == Qt::DecorationRole ) {
        return KIcon( QString::fromLatin1( "text-plain" ) ).pixmap(42);
    }

    return QVariant();
}

QVariant Browser::OverviewModel::searchInfo( int role ) const
{
    if ( role == Qt::DisplayRole )
        return i18n("Search");
    else if ( role == Qt::DecorationRole )
        return KIcon( QString::fromLatin1( "system-search" ) ).pixmap(42);
    return QVariant();
}

QVariant Browser::OverviewModel::imageInfo( int role ) const
{
    if ( role == Qt::DisplayRole )
        return i18n("Show Thumbnails");
    else if ( role == Qt::DecorationRole )
        return KIcon( QString::fromLatin1( "kphotoalbum" ) ).pixmap(42);
    return QVariant();
}

void Browser::OverviewModel::action( const QModelIndex& index )
{
    const int row = index.row();

    if ( isCategoryIndex(row) )
        executeCategoryAction( row );
    else if ( isExivIndex( row ) )
        return executeExivAction();
    else if ( isSearchIndex( row ) )
        return executeSearchAction();
    else if ( isImageIndex( row ) )
        return executeImageAction( );
}

void Browser::OverviewModel::executeCategoryAction( int row ) const
{
    Browser::CategoryModel* model = new Browser::CategoryModel( categories()[row], _info, browser() );
    browser()->addModel( model );
}

void Browser::OverviewModel::executeExivAction() const
{
    // PENDING(blackie)
}

void Browser::OverviewModel::executeSearchAction() const
{
    // PENDING(blackie) implement
}

void Browser::OverviewModel::executeImageAction() const
{
    MainWindow::Window::theMainWindow()->showThumbNails( DB::ImageDB::instance()->search( _info ) );

// PENDING(blackie) _context comes from jumpToContext - see ImageFolder.cpp
#ifdef KDAB_TEMPORARILY_REMOVED
    if ( !_context.isNull() ) {
        DB::ResultId id = DB::ImageDB::instance()->ID_FOR_FILE( _context );
        ThumbnailView::ThumbnailWidget::theThumbnailView()->setCurrentItem( id );
    }
#else // KDAB_TEMPORARILY_REMOVED
    qWarning("Code commented out in Browser::OverviewModel::executeImageAction");
#endif //KDAB_TEMPORARILY_REMOVED

}

QAbstractListModel* Browser::OverviewModel::model()
{
    return this;
}

