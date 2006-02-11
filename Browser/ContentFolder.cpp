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

#include "ContentFolder.h"
#include "options.h"
#include "TypeFolder.h"
#include "ImageFolder.h"
#include <klocale.h>
#include "imagedb.h"
#include "SearchFolder.h"
#include <kglobal.h>
#include <kiconloader.h>
#include "BrowserItemFactory.h"
#include "categorycollection.h"
#include "membermap.h"
#include "ExifFolder.h"
#include "Exif/Database.h"
#include <config.h>

Browser::ContentFolder::ContentFolder( const QString& category, const QString& value, int count,
                              const ImageSearchInfo& info, Browser* parent )
    :Folder( info, parent ), _category( category ), _value( value )
{
    _info.addAnd( _category, _value );
    setCount( count );
}

QPixmap Browser::ContentFolder::pixmap()
{
    if ( ImageDB::instance()->categoryCollection()->categoryForName( _category )->viewSize() == Category::Small ) {
        if ( ImageDB::instance()->memberMap().isGroup( _category, _value ) )
            return KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "kuser" ), KIcon::Desktop, 22 );
        else {
            return ImageDB::instance()->categoryCollection()->categoryForName( _category )->icon();
        }
    }
    else
        return Options::instance()->categoryImage( _category, _value, 64 );
}

QString Browser::ContentFolder::text() const
{
    if ( _value == ImageDB::NONE() ) {
        if ( _info.option(_category) == ImageDB::NONE() )
            return i18n( "No %1" ).arg( ImageDB::instance()->categoryCollection()->categoryForName( _category )->text() );
        else
            return i18n( "No other %1" ).arg( ImageDB::instance()->categoryCollection()->categoryForName( _category )->text() );
    }
    else {
        return _value;
    }
}


void Browser::ContentFolderAction::action( BrowserItemFactory* factory )
{
    _browser->clear();
    QStringList grps = ImageDB::instance()->categoryCollection()->categoryNames();

    for( QStringList::Iterator it = grps.begin(); it != grps.end(); ++it ) {
        factory->createItem( new TypeFolder( *it, _info, _browser ) );
    }

    //-------------------------------------------------- Search,Exif, and Image Folder
    factory->createItem( new SearchFolder( _info, _browser ) );
#ifdef HASEXIV2
    if ( Exif::Database::isAvailable() )
        factory->createItem( new ExifFolder( _info, _browser ) );
#endif
    factory->createItem( new ImageFolder( _info, _browser ) );
}

Browser::FolderAction* Browser::ContentFolder::action( bool ctrlDown )
{
    bool loadImages = (ImageDB::instance()->count( _info ) < Options::instance()->autoShowThumbnailView());
    if ( ctrlDown ) loadImages = !loadImages;

    if ( loadImages ) {
        ImageSearchInfo info = _info;
        return new ImageFolderAction( info, _browser );
    }

    return new ContentFolderAction( _category, _value, _info, _browser );
}

Browser::ContentFolderAction::ContentFolderAction( const QString& category, const QString& value,
                                          const ImageSearchInfo& info, Browser* browser )
    :FolderAction( info, browser ), _category( category ), _value( value )
{
}

int Browser::ContentFolder::compare( Folder* other, int col, bool asc ) const
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

bool Browser::ContentFolderAction::allowSort() const
{
    return false;
}


QString Browser::ContentFolderAction::title() const
{
    return i18n("Category");
}

QString Browser::ContentFolder::countLabel() const
{
    return i18n("1 Image", "%n Images", _count);
}
