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

#include "contentfolder.h"
#include "options.h"
#include "typefolder.h"
#include "imagefolder.h"
#include <klocale.h>
#include "imagedb.h"
#include "searchfolder.h"
#include "datefolder.h"
#include <kglobal.h>
#include <kiconloader.h>
#include "browseritemfactory.h"
#include "categorycollection.h"
ContentFolder::ContentFolder( const QString& category, const QString& value, int count,
                              const ImageSearchInfo& info, Browser* parent )
    :Folder( info, parent ), _category( category ), _value( value )
{
    _info.addAnd( _category, _value );
    setCount( count );
}

QPixmap ContentFolder::pixmap()
{
    if ( CategoryCollection::instance()->categoryForName( _category )->viewSize() == Category::Small ) {
        if ( Options::instance()->memberMap().isGroup( _category, _value ) )
            return KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "kuser" ), KIcon::Desktop, 22 );
        else {
            return CategoryCollection::instance()->categoryForName( _category )->icon();
        }
    }
    else
        return Options::instance()->optionImage( _category, _value, 64 );
}

QString ContentFolder::text() const
{
    if ( _value == ImageDB::NONE() ) {
        if ( _info.option(_category) == ImageDB::NONE() )
            return i18n( "No %1" ).arg( CategoryCollection::instance()->categoryForName( _category )->text() );
        else
            return i18n( "No other %1" ).arg( CategoryCollection::instance()->categoryForName( _category )->text() );
    }
    else {
        return _value;
    }
}


void ContentFolderAction::action( BrowserItemFactory* factory )
{
    _browser->clear();
    QStringList grps = CategoryCollection::instance()->categoryNames();

    for( QStringList::Iterator it = grps.begin(); it != grps.end(); ++it ) {
        factory->createItem( new TypeFolder( *it, _info, _browser ) );
    }

    //-------------------------------------------------- Search Folders
    factory->createItem( new DateFolder( _info, _browser ) );
    factory->createItem( new SearchFolder( _info, _browser ) );

    //-------------------------------------------------- Image Folders
    int count = ImageDB::instance()->count( _info );
    int maxPerPage = Options::instance()->maxImages();

    if ( count < maxPerPage ) {
        factory->createItem( new ImageFolder( _info, _browser ) );
    }
    else {
        int last = 1;
        while ( last < count ) {
            factory->createItem( new ImageFolder( _info, last, QMIN( count, last+maxPerPage-1 ), _browser ) );
            last += maxPerPage;
        }
    }
}

FolderAction* ContentFolder::action( bool ctrlDown )
{
    bool loadImages = ImageDB::instance()->count( _info ) < Options::instance()->autoShowThumbnailView();
    if ( ctrlDown ) loadImages = !loadImages;

    if ( loadImages ) {
        ImageSearchInfo info = _info;
        if ( ImageDB::instance()->count( info ) < Options::instance()->maxImages() )
        if ( ImageDB::instance()->count( info ) < Options::instance()->maxImages() )
            return new ImageFolderAction( info, -1, -1, _browser );
    }

    return new ContentFolderAction( _category, _value, _info, _browser );
}

ContentFolderAction::ContentFolderAction( const QString& category, const QString& value,
                                          const ImageSearchInfo& info, Browser* browser )
    :FolderAction( info, browser ), _category( category ), _value( value )
{
}

int ContentFolder::compare( Folder* other, int col, bool asc ) const
{
    if ( col == 0 ) {
        if ( _value == ImageDB::NONE() )
            return ( asc ? -1 : 1);
        ContentFolder* o = static_cast<ContentFolder*>( other );
        if ( o->_value == ImageDB::NONE() )
            return ( asc ? 1: -1 );
    }

    return Folder::compare( other, col, asc );
}

bool ContentFolderAction::allowSort() const
{
    return false;
}


QString ContentFolderAction::title() const
{
    return i18n("Category");
}

QString ContentFolder::countLabel() const
{
    return i18n("1 Image", "%n Images", _count);
}
