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

#include "typefolder.h"
#include "options.h"
#include "imagedb.h"
#include "contentfolder.h"
#include <klocale.h>
#include "browseritemfactory.h"
#include "categorycollection.h"

TypeFolder::TypeFolder( const QString& category, const ImageSearchInfo& info, Browser* parent )
    :Folder( info, parent ), _category ( category )
{
    QMap<QString, int> map = ImageDB::instance()->classify( _info, _category );
    int count = map.size();
    setCount( count );
    if ( count <= 1 )
        setEnabled( false );
}

QPixmap TypeFolder::pixmap()
{
    return CategoryCollection::instance()->categoryForName( _category )->icon();
}

QString TypeFolder::text() const
{
    return CategoryCollection::instance()->categoryForName( _category )->text();
}

FolderAction* TypeFolder::action( bool /* ctrlDown */ )
{
    return new TypeFolderAction( _category, _info, _browser );
}

TypeFolderAction::TypeFolderAction( const QString& category, const ImageSearchInfo& info,
                                    Browser* browser )
    :FolderAction( info, browser ), _category( category )
{
}

void TypeFolderAction::action( BrowserItemFactory* factory )
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

QString TypeFolderAction::title() const
{
    return CategoryCollection::instance()->categoryForName( _category )->text();
}

QString TypeFolderAction::category() const
{
    return _category;
}

QString TypeFolder::countLabel() const
{
    return i18n("1 Category", "%n Categories", _count);
}

bool TypeFolderAction::contentView() const
{
    return ( !CategoryCollection::instance()->categoryForName( _category )->isSpecialCategory() );
}



