#include "ExifFolder.h"
#include <kiconloader.h>
#include <kglobal.h>
#include <klocale.h>
#include "Exif/SearchDialog.h"
#include "ContentFolder.h"
#include "DB/ImageDB.h"
#include <kmessagebox.h>
#include "ContentFolderAction.h"

Browser::ExifFolder::ExifFolder( const DB::ImageSearchInfo& info, BrowserWidget* browser )
    :Folder( info, browser )
{
}


Browser::FolderAction* Browser::ExifFolder::action( bool /* ctrlDown */ )
{
    Exif::SearchDialog dialog( _browser );
    if ( dialog.exec() == QDialog::Rejected )
        return 0;

    Exif::SearchInfo result = dialog.info();

    DB::ImageSearchInfo info = _info;

    info.addExifSearchInfo( dialog.info() );

    if ( DB::ImageDB::instance()->count( info ).total() == 0 ) {
        KMessageBox::information( _browser, i18n( "Search did not match any images." ), i18n("Empty Search Result") );
        return 0;
    }

    return new ContentFolderAction( info, _browser );
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


QString Browser::ExifFolder::imagesLabel() const
{
        return QString::null;
}

QString Browser::ExifFolder::videosLabel() const
{
        return QString::null;
}
