#include "imagedb.h"
#include "xmldb/xmldb.h"
#include <klocale.h>
#include <qfileinfo.h>
#include "browser.h"
#include "categorycollection.h"
#include "sqldb/sqldb.h"

ImageDB* ImageDB::_instance = 0;


ImageDB* ImageDB::instance()
{
    if ( _instance == 0 )
        qFatal("ImageDB::instance must not be called before ImageDB::setup");
    return _instance;
}

void ImageDB::setup( const QString& backend, const QString& configFile )
{
    if ( backend == QString::fromLatin1( "sql" ) )
        _instance = new SQLDB::SQLDB;
    else
        _instance = new XMLDB::XMLDB( configFile );
    connect( _instance->categoryCollection(), SIGNAL( itemRemoved( Category*, const QString& ) ),
             _instance, SLOT( deleteItem( Category*, const QString& ) ) );
    connect( _instance->categoryCollection(), SIGNAL( itemRenamed( Category*, const QString&, const QString& ) ),
             _instance, SLOT( renameItem( Category*, const QString&, const QString& ) ) );
    connect( Options::instance(), SIGNAL( locked( bool, bool ) ), _instance, SLOT( lockDB( bool, bool ) ) );
}

QString ImageDB::NONE()
{
    return i18n("**NONE**");
}

QStringList ImageDB::currentScope( bool requireOnDisk ) const
{
    return search( Browser::instance()->currentContext(), requireOnDisk );
}

void ImageDB::setDateRange( const ImageDateRange& range, bool includeFuzzyCounts )
{
    _selectionRange = range;
    _includeFuzzyCounts = includeFuzzyCounts;
}

void ImageDB::clearDateRange()
{
    _selectionRange = ImageDateRange();
}

ImageInfoList ImageDB::clipboard()
{
    return _clipboard;
}

void ImageDB::setClipboard( const ImageInfoList& list )
{
    _clipboard = list;
}

bool ImageDB::isClipboardEmpty()
{
    return (_clipboard.count() == 0 );
}

void ImageDB::slotRescan()
{
    bool newImages = NewImageFinder().findImages();
    if ( newImages )
        emit dirty();

    emit totalChanged( totalCount() );
}

void ImageDB::slotRecalcCheckSums()
{
    md5Map()->clear();
    bool d = NewImageFinder().calculateMD5sums( images() );
    if ( d )
        emit dirty();

    // To avoid deciding if the new images are shown in a given thumbnail view or in a given search
    // we rather just go to home.
    Browser::instance()->home();

    emit totalChanged( totalCount() );
}

ImageDB::ImageDB()
{
}

int ImageDB::count( const ImageSearchInfo& info )
{
    int count = search( info ).count();
    return count;
}

