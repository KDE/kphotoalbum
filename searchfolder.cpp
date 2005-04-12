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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "searchfolder.h"
#include <klocale.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include "imageconfig.h"
#include "contentfolder.h"
#include "imagedb.h"
#include <kmessagebox.h>
#include <kglobal.h>
#include "imageinfo.h"

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
        KMessageBox::information( _browser, i18n( "Search did not match any images." ), i18n("Empty Search Result") );
        return 0;
    }

    return new ContentFolderAction( QString::null, QString::null, info, _browser );
}

QString SearchFolder::countLabel() const
{
    return QString::null;
}
