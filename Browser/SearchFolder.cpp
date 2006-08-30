/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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

#include "SearchFolder.h"
#include <klocale.h>
#include <kiconloader.h>
#include "AnnotationDialog/Dialog.h"
#include "ContentFolder.h"
#include "DB/ImageDB.h"
#include <kmessagebox.h>
#include <kglobal.h>
#include "ContentFolderAction.h"

Browser::SearchFolder::SearchFolder( const DB::ImageSearchInfo& info, BrowserWidget* browser )
    :Folder( info, browser )
{
}

QPixmap Browser::SearchFolder::pixmap()
{
    KIconLoader loader;
    return KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "find" ), KIcon::Desktop, 22 );
}

QString Browser::SearchFolder::text() const
{
    return i18n("Search");
}


Browser::FolderAction* Browser::SearchFolder::action( bool )
{
    AnnotationDialog::Dialog config( _browser );
    DB::ImageSearchInfo info = config.search( &_info );
    if ( info.isNull() )
        return 0;

    if ( DB::ImageDB::instance()->count( info ).total() == 0 ) {
        KMessageBox::information( _browser, i18n( "Search did not match any images." ), i18n("Empty Search Result") );
        return 0;
    }

    return new ContentFolderAction( info, _browser );
}

QString Browser::SearchFolder::imagesLabel() const
{
    return QString::null;
}
QString Browser::SearchFolder::videosLabel() const
{
    return QString::null;
}
