#include "searchfolder.h"
#include <klocale.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include "imageconfig.h"
#include "contentfolder.h"
#include "imagedb.h"
#include <kmessagebox.h>

SearchFolder::SearchFolder( const ImageSearchInfo& info, Browser* browser )
    :Folder( info, browser )
{
    setText( i18n("Search") );
    KIconLoader loader;
    setPixmap( locate("data", QString::fromLatin1("kimdaba/pics/searchIcon.png") ) );
}

FolderAction* SearchFolder::action( bool )
{
    ImageConfig config( _browser );
    ImageSearchInfo info = config.search( &_info );
    if ( info.isNull() )
        return 0;

    if ( ImageDB::instance()->count( info ) == 0 ) {
        KMessageBox::information( _browser, i18n( "Search didn't match any images" ), i18n("Empty search result") );
        return 0;
    }

    return new ContentFolderAction( QString::null, QString::null, info, _browser );
}
