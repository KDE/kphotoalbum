#include "ContentFolderAction.h"
#include "Settings/SettingsData.h"
#include "TypeFolder.h"
#include "ImageFolder.h"
#include <klocale.h>
#include "DB/ImageDB.h"
#include "SearchFolder.h"
#include "BrowserItemFactory.h"
#include "DB/CategoryCollection.h"
#include "ExifFolder.h"
#include "Exif/Database.h"
#include <config.h> // for HASEXIV2

void Browser::ContentFolderAction::action( BrowserItemFactory* factory )
{
    _browser->clear();

    const QValueList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    for( QValueList<DB::CategoryPtr>::ConstIterator categoryIt = categories.begin(); categoryIt != categories.end(); ++categoryIt ) {
        factory->createItem( new TypeFolder( *categoryIt, _info, _browser ), 0 );
    }

    //-------------------------------------------------- Search,Exif, and Image Folder
    factory->createItem( new SearchFolder( _info, _browser), 0 );
#ifdef HASEXIV2
    if ( Exif::Database::isAvailable() )
        factory->createItem( new ExifFolder( _info, _browser ), 0 );
#endif
    factory->createItem( new ImageFolder( _info, _browser), 0 );
}

Browser::ContentFolderAction::ContentFolderAction( const DB::ImageSearchInfo& info, BrowserWidget* browser )
    :FolderAction( info, browser )
{
}

bool Browser::ContentFolderAction::allowSort() const
{
    return false;
}


QString Browser::ContentFolderAction::title() const
{
    return i18n("Category");
}

