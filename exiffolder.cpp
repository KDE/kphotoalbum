#include "exiffolder.h"
#include <kiconloader.h>
#include <kglobal.h>
#include <klocale.h>
#include "Exif/exifsearchdialog.h"

ExifFolder::ExifFolder( const ImageSearchInfo& info, Browser* browser )
    :Folder( info, browser )
{
}


FolderAction* ExifFolder::action( bool /* ctrlDown */ )
{
    ExifSearchDialog dialog( _browser );
    dialog.exec();
    QStringList list = dialog.info().matches();
    qDebug( "%s", list.join( QString::fromLatin1( ", " ) ).latin1() );
    return 0;
}


QPixmap ExifFolder::pixmap()
{
    KIconLoader loader;
    return KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "contents" ), KIcon::Desktop, 22 );
}


QString ExifFolder::text() const
{
    return i18n("Exif Info");
}


QString ExifFolder::countLabel() const
{
        return QString::null;
}
