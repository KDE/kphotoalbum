/*
 *  Copyright (c) 2003-2004 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

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
ContentFolder::ContentFolder( const QString& optionGroup, const QString& value, int count,
                              const ImageSearchInfo& info, Browser* parent )
    :Folder( info, parent ), _optionGroup( optionGroup ), _value( value )
{
    if ( value == ImageDB::NONE() ) {
        _info.setOption( _optionGroup, ImageDB::NONE() );
    }
    else if ( !_optionGroup.isNull() ) {
        // It will be null for the initial element ceated from the browser.
        _info.addAnd( _optionGroup, _value );
    }
    setCount( count );
}

QPixmap ContentFolder::pixmap()
{
    if ( Options::instance()->viewSize( _optionGroup ) == Options::Small ) {
        if ( Options::instance()->memberMap().isGroup( _optionGroup, _value ) )
            return KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "kuser" ), KIcon::Desktop, 22 );
        else {
            return Options::instance()->iconForOptionGroup( _optionGroup );
        }
    }
    else
        return Options::instance()->optionImage( _optionGroup, _value, 64 );
}

QString ContentFolder::text() const
{
    if ( _value == ImageDB::NONE() ) {
        return i18n( "None" );
    }
    else {
        return _value;
    }
}


void ContentFolderAction::action( BrowserItemFactory* factory )
{
    _browser->clear();
    QStringList grps = Options::instance()->optionGroups();

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
    if ( ctrlDown ) {
        ImageSearchInfo info = _info;
        info.addAnd( _optionGroup, _value );
        if ( ImageDB::instance()->count( info ) < Options::instance()->maxImages() )
            return new ImageFolderAction( info, -1, -1, _browser );
    }

    return new ContentFolderAction( _optionGroup, _value, _info, _browser );
}

ContentFolderAction::ContentFolderAction( const QString& optionGroup, const QString& value,
                                          const ImageSearchInfo& info, Browser* browser )
    :FolderAction( info, browser ), _optionGroup( optionGroup ), _value( value )
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
