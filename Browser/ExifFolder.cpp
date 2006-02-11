#include "ExifFolder.h"
#include <kiconloader.h>
#include <kglobal.h>
#include <klocale.h>
#include "Exif/SearchDialog.h"
#include "ContentFolder.h"
#include "imagedb.h"
#include <kmessagebox.h>

Browser::ExifFolder::ExifFolder( const ImageSearchInfo& info, Browser* browser )
    :Folder( info, browser )
{
}


Browser::FolderAction* Browser::ExifFolder::action( bool /* ctrlDown */ )
{
    Exif::SearchDialog dialog( _browser );
    if ( dialog.exec() == QDialog::Rejected )
        return 0;

    Exif::SearchInfo result = dialog.info();

    ImageSearchInfo info = _info;

    info.addExifSearchInfo( dialog.info() );

    if ( ImageDB::instance()->count( info ) == 0 ) {
        KMessageBox::information( _browser, i18n( "Search did not match any images." ), i18n("Empty Search Result") );
        return 0;
    }

    return new ContentFolderAction( QString::null, QString::null, info, _browser );
}


QPixmap Browser::ExifFolder::pixmap()
{
    KIconLoader loader;
    return KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "contents" ), KIcon::Desktop, 22 );
}


QString Browser::ExifFolder::text() const
{
    return i18n("Exif Info");
}


QString Browser::ExifFolder::countLabel() const
{
        return QString::null;
}
