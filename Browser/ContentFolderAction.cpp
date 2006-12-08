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

