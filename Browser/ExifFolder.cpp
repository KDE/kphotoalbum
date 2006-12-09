/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
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
        KMessageBox::information( _browser, i18n( "Search did not match any images or videos." ), i18n("Empty Search Result") );
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
