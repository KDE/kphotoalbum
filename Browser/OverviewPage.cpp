#include "OverviewPage.h"
#include "enums.h"
#include <KMessageBox>
#include <Exif/SearchDialog.h>
#include "ImageViewPage.h"
#include "CategoryPage.h"
#include "BrowserWidget.h"
#include <MainWindow/Window.h>
#include <QDebug>
#include <klocale.h>
#include <DB/ImageDB.h>
#include <kicon.h>
#include <config-kpa-exiv2.h>

AnnotationDialog::Dialog* Browser::OverviewPage::_config = 0;
Browser::OverviewPage::OverviewPage( const Breadcrumb& breadcrumb, const DB::ImageSearchInfo& info, BrowserWidget* browser )
    : BrowserPage( info, browser), _breadcrumb( breadcrumb )
{
    int row = 0;
    Q_FOREACH( const DB::CategoryPtr& category, categories() ) {
        QMap<QString, uint> images = DB::ImageDB::instance()->classify( BrowserPage::searchInfo(), category->name(), DB::Image );
        QMap<QString, uint> videos = DB::ImageDB::instance()->classify( BrowserPage::searchInfo(), category->name(), DB::Video );
        DB::MediaCount count( images.count(), videos.count() );
        _count[row] = count;
        ++row;
    }
}

int Browser::OverviewPage::rowCount( const QModelIndex& parent ) const
{
    if ( parent != QModelIndex() )
        return 0;

    return categories().count() +
#ifdef HAVE_EXIV2
        1 +
#endif
        2; // Search info + Show Image
}

QVariant Browser::OverviewPage::data( const QModelIndex& index, int role) const
{
    if ( role == ValueRole )
        return index.row();

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

bool Browser::OverviewPage::isCategoryIndex( int row ) const
{
    return row < categories().count();
}

bool Browser::OverviewPage::isExivIndex( int row ) const
{
#ifdef HAVE_EXIV2
    return row == categories().count();
#else
    Q_UNUSED(row);
    return false;
#endif
}

bool Browser::OverviewPage::isSearchIndex( int row ) const
{
    return rowCount()-2 == row;
}

bool Browser::OverviewPage::isImageIndex( int row ) const
{
    return rowCount()-1 == row;
}


QList<DB::CategoryPtr> Browser::OverviewPage::categories() const
{
    return DB::ImageDB::instance()->categoryCollection()->categories();
}

QVariant Browser::OverviewPage::categoryInfo( int row, int role ) const
{
    if ( role == Qt::DisplayRole )
        return categories()[row]->text();
    else if ( role == Qt::DecorationRole )
        return categories()[row]->icon(42);

    return QVariant();
}

QVariant Browser::OverviewPage::exivInfo( int role ) const
{
    if ( role == Qt::DisplayRole )
        return i18n("Exif Info");
    else if ( role == Qt::DecorationRole ) {
        return KIcon( QString::fromLatin1( "text-plain" ) ).pixmap(42);
    }

    return QVariant();
}

QVariant Browser::OverviewPage::searchInfo( int role ) const
{
    if ( role == Qt::DisplayRole )
        return i18n("Search");
    else if ( role == Qt::DecorationRole )
        return KIcon( QString::fromLatin1( "system-search" ) ).pixmap(42);
    return QVariant();
}

QVariant Browser::OverviewPage::imageInfo( int role ) const
{
    if ( role == Qt::DisplayRole )
        return i18n("Show Thumbnails");
    else if ( role == Qt::DecorationRole )
        return KIcon( QString::fromLatin1( "kphotoalbum" ) ).pixmap(42);
    return QVariant();
}

Browser::BrowserPage* Browser::OverviewPage::activateChild( const QModelIndex& index )
{
    const int row = index.row();

    if ( isCategoryIndex(row) )
        return new Browser::CategoryPage( categories()[row], BrowserPage::searchInfo(), browser() );
    else if ( isExivIndex( row ) )
        return activateExivAction();
    else if ( isSearchIndex( row ) )
        return activateSearchAction();
    else if ( isImageIndex( row ) )
        return new ImageViewPage( BrowserPage::searchInfo(), browser()  );

    return 0;
}

void Browser::OverviewPage::activate()
{
    browser()->setModel( this );
}

Qt::ItemFlags Browser::OverviewPage::flags( const QModelIndex & index ) const
{
    if ( isCategoryIndex(index.row() ) && _count[index.row()].total() <= 1 )
        return QAbstractListModel::flags(index) & ~Qt::ItemIsEnabled;
    else
        return QAbstractListModel::flags(index);
}

bool Browser::OverviewPage::isSearchable() const
{
    return true;
}

Browser::BrowserPage* Browser::OverviewPage::activateExivAction()
{
#ifdef HAVE_EXIV2
    Exif::SearchDialog dialog( browser() );
    if ( dialog.exec() == QDialog::Rejected )
        return 0;

    Exif::SearchInfo result = dialog.info();

    DB::ImageSearchInfo info = BrowserPage::searchInfo();

    info.addExifSearchInfo( dialog.info() );

    if ( DB::ImageDB::instance()->count( info ).total() == 0 ) {
        KMessageBox::information( browser(), i18n( "Search did not match any images or videos." ), i18n("Empty Search Result") );
        return 0;
    }

    return new OverviewPage( Breadcrumb( i18n("EXIF Search")), info, browser() );
#else
    return 0;
#endif // HAVE_EXIV2
}

Browser::BrowserPage* Browser::OverviewPage::activateSearchAction()
{
    if ( !_config )
        _config = new AnnotationDialog::Dialog( browser() );

    DB::ImageSearchInfo tmpInfo = BrowserPage::searchInfo();
    DB::ImageSearchInfo info = _config->search( &tmpInfo ); // PENDING(blackie) why take the address?

    if ( info.isNull() )
        return 0;

    if ( DB::ImageDB::instance()->count( info ).total() == 0 ) {
        KMessageBox::information( browser(), i18n( "Search did not match any images or videos." ), i18n("Empty Search Result") );
        return 0;
    }

    return new OverviewPage( Breadcrumb( i18n("search") ), info, browser() );

}

Browser::Breadcrumb Browser::OverviewPage::breadcrumb() const
{
    return _breadcrumb;
}

bool Browser::OverviewPage::showDuringMovement() const
{
    return true;
}

