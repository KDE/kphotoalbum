#include "searchfolder.h"
#include <klocale.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include "imageconfig.h"
#include "contentfolder.h"
#include "imagedb.h"
#include <kmessagebox.h>
#include <kglobal.h>

SearchFolder::SearchFolder( const ImageSearchInfo& info, Browser* browser )
    :Folder( info, browser )
{
    setCount(-1);
}

QPixmap SearchFolder::pixmap()
{
    KIconLoader loader;
    return KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "find" ), KIcon::Desktop, 22 );
}

QString SearchFolder::text() const
{
    return i18n("Search");
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

QString SearchFolder::countLabel() const
{
    return QString::null;
}
