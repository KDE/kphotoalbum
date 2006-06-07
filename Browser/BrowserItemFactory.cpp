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

#include "BrowserItemFactory.h"
#include "Folder.h"
#include <klocale.h>

Browser::BrowserIconViewItemFactory::BrowserIconViewItemFactory( QIconView* view )
    :BrowserItemFactory(), _view( view )
{
}

void Browser::BrowserIconViewItemFactory::createItem( Folder* folder )
{
    if ( folder->text().lower().contains( _matchText.lower() ) )
        new BrowserIconItem( _view, folder );
}

Browser::BrowserListViewItemFactory::BrowserListViewItemFactory( QListView* view )
    :BrowserItemFactory(), _view( view )
{
}

void Browser::BrowserListViewItemFactory::createItem( Folder* folder )
{
    BrowserListItem* item = new BrowserListItem( _view, folder );
    item->setEnabled( folder->_enabled );
}

Browser::BrowserIconItem::BrowserIconItem( QIconView* view, Folder* folder )
    :QIconViewItem( view ), _folder(folder)
{
    setPixmap( folder->pixmap() );
    DB::MediaCount count = folder->count();
    if ( count.images() == -1 )
        setText( folder->text() );
    else
        setText( QString::fromLatin1( "%1 (%2/%3)" ).arg( folder->text() ).arg( count.images() ).arg( count.movies() ));
}

Browser::BrowserListItem::BrowserListItem( QListView* view, Folder* folder )
     : QListViewItem( view ), _folder(folder)
{
    setPixmap( 0, folder->pixmap() );
    setText( 0, folder->text() );
    setText( 1, folder->imagesLabel() );
    setText( 2, folder->moviesLabel() );
}

int Browser::BrowserListItem::compare( QListViewItem* other, int col, bool asc ) const
{
    return _folder->compare( static_cast<BrowserListItem*>(other)->_folder, col, asc );
}

Browser::BrowserIconItem::~BrowserIconItem()
{
    delete _folder;
}

Browser::BrowserListItem::~BrowserListItem()
{
    delete _folder;
}

void Browser::BrowserIconViewItemFactory::setMatchText( const QString& text )
{
    _matchText = text;
}

