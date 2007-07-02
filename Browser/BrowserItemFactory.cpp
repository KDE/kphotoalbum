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

#include "BrowserItemFactory.h"
#include "Folder.h"

Browser::BrowserIconViewItemFactory::BrowserIconViewItemFactory( Q3IconView* view )
    :BrowserItemFactory(), _view( view )
{
}

Browser::BrowserItem* Browser::BrowserIconViewItemFactory::createItem( Folder* folder, BrowserItem* /*parent*/ )
{
    if ( folder->text().toLower().contains( _matchText.toLower() ) )
        return new BrowserIconItem( _view, folder );
    else
        return 0;
}

Browser::BrowserListViewItemFactory::BrowserListViewItemFactory( Q3ListView* view )
    :BrowserItemFactory(), _view( view )
{
}

Browser::BrowserItem* Browser::BrowserListViewItemFactory::createItem( Folder* folder, BrowserItem* parent )
{
    BrowserListItem* item;
    if ( parent )
        item = new BrowserListItem( dynamic_cast<BrowserListItem*>( parent ), folder);
    else
        item = new BrowserListItem( _view, folder );
    item->setEnabled( folder->_enabled );
    return item;
}

Browser::BrowserIconItem::BrowserIconItem( Q3IconView* view, Folder* folder )
    :Q3IconViewItem( view ), _folder(folder)
{
    setPixmap( folder->pixmap() );
    DB::MediaCount count = folder->count();
    if ( count.isNull() )
        setText( folder->text() );
    else
        setText( QString::fromLatin1( "%1 (%2/%3)" ).arg( folder->text() ).arg( count.images() ).arg( count.videos() ));
}

Browser::BrowserListItem::BrowserListItem( Q3ListView* view, Folder* folder )
     : Q3ListViewItem( view ), _folder(folder)
{
    setPixmap( 0, folder->pixmap() );
    setText( 0, folder->text() );
    setText( 1, folder->imagesLabel() );
    setText( 2, folder->videosLabel() );
    setOpen( true );
}

Browser::BrowserListItem::BrowserListItem( Q3ListViewItem* item, Folder* folder )
     : Q3ListViewItem( item ), _folder(folder)
{
    setPixmap( 0, folder->pixmap() );
    setText( 0, folder->text() );
    setText( 1, folder->imagesLabel() );
    setText( 2, folder->videosLabel() );
    setOpen( true );
}


int Browser::BrowserListItem::compare( Q3ListViewItem* other, int col, bool asc ) const
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

bool Browser::BrowserIconViewItemFactory::supportsHierarchy() const
{
    return false;
}

bool Browser::BrowserListViewItemFactory::supportsHierarchy() const
{
    return true;
}

