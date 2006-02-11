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

#include "TypeFolder.h"
#include "options.h"
#include "imagedb.h"
#include "ContentFolder.h"
#include <klocale.h>
#include "BrowserItemFactory.h"
#include "categorycollection.h"

Browser::TypeFolder::TypeFolder( const QString& category, const ImageSearchInfo& info, Browser* parent )
    :Folder( info, parent ), _category ( category )
{
    QMap<QString, int> map = ImageDB::instance()->classify( _info, _category );
    int count = map.size();
    setCount( count );
    if ( count <= 1 )
        setEnabled( false );
}

QPixmap Browser::TypeFolder::pixmap()
{
    return ImageDB::instance()->categoryCollection()->categoryForName( _category )->icon();
}

QString Browser::TypeFolder::text() const
{
    return ImageDB::instance()->categoryCollection()->categoryForName( _category )->text();
}

Browser::FolderAction* Browser::TypeFolder::action( bool /* ctrlDown */ )
{
    return new TypeFolderAction( _category, _info, _browser );
}

Browser::TypeFolderAction::TypeFolderAction( const QString& category, const ImageSearchInfo& info,
                                    Browser* browser )
    :FolderAction( info, browser ), _category( category )
{
}

void Browser::TypeFolderAction::action( BrowserItemFactory* factory )
{
    _browser->clear();


    QMap<QString, int> map = ImageDB::instance()->classify( _info, _category );
    for( QMapIterator<QString,int> it= map.begin(); it != map.end(); ++it ) {
        if ( it.key() != ImageDB::NONE() ) {
            factory->createItem( new ContentFolder( _category, it.key(), it.data(), _info, _browser ) );
        }
    }

    // Add the none option to the end
    int i = map[ImageDB::NONE()];
    if ( i != 0 )
        factory->createItem( new ContentFolder( _category, ImageDB::NONE(), i, _info, _browser ) );
}

QString Browser::TypeFolderAction::title() const
{
    return ImageDB::instance()->categoryCollection()->categoryForName( _category )->text();
}

QString Browser::TypeFolderAction::category() const
{
    return _category;
}

QString Browser::TypeFolder::countLabel() const
{
    return i18n("1 Category", "%n Categories", _count);
}

bool Browser::TypeFolderAction::contentView() const
{
    return ( !ImageDB::instance()->categoryCollection()->categoryForName( _category )->isSpecialCategory() );
}



