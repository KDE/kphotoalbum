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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
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
//Added by qt3to4:
#include <QPixmap>

Browser::SearchFolder::SearchFolder( const DB::ImageSearchInfo& info, BrowserWidget* browser )
    :Folder( info, browser ), _config(0)
{
}

QPixmap Browser::SearchFolder::pixmap()
{
    return KIcon( QString::fromLatin1( "system-search" ) ).pixmap(22);
}

QString Browser::SearchFolder::text() const
{
    return i18n("Search");
}


Browser::FolderAction* Browser::SearchFolder::action( bool )
{
    if ( !_config )
        _config = new AnnotationDialog::Dialog( _browser );
    DB::ImageSearchInfo info = _config->search( &_info );

    if ( info.isNull() )
        return 0;

    if ( DB::ImageDB::instance()->count( info ).total() == 0 ) {
        KMessageBox::information( _browser, i18n( "Search did not match any images or videos." ), i18n("Empty Search Result") );
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
