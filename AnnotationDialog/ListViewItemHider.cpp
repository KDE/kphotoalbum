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
#include "ListViewItemHider.h"

/**
 * \class AnnotationDialog::ListViewHider
 * \brief Helper class, used to hide/show listview items
 *
 * This is a helper class that is used to hide items in a listview. A leaf
 * will be hidden if then subclass implemented method \ref
 * shouldItemBeShown returns true for the given item. A parent node is
 * hidden if none of the children are shown, and \ref shouldItemBeShown
 * also returns false for the parent itself.
 */

/**
 * \class AnnotationDialog::ListViewTextMatchHider
 * \brief Helper class for showing items matching a given text string.
 */

/**
 * \class AnnotationDialog::ListViewCheckedHider
 * \brief Helper class for only showing items that are selected.
 */

void AnnotationDialog::ListViewHider::setItemsVisible( QListView* listView )
{
    // It seems like a bug in Qt, but I need to make all item visible first, otherwise I see wrong items when I have items
    // two layers deep (A->B->c). It only occours when I widen (types "je", now deletes the "e" to get to "j" ).
    for ( QListViewItemIterator it( listView ); *it; ++it )
        (*it)->setVisible( true );

    for ( QListViewItem* item = listView->firstChild(); item; item = item->nextSibling() ) {
        bool anyChildrenVisible = setItemsVisible( item );
        bool visible = anyChildrenVisible || shouldItemBeShown( item );
        item->setVisible( visible );
    }
}

bool AnnotationDialog::ListViewHider::setItemsVisible( QListViewItem* parentItem )
{
    bool anyChildrenVisible = false;
    for ( QListViewItem* item = parentItem->firstChild(); item; item = item->nextSibling() ) {
        bool anySubChildrenVisible = setItemsVisible( item );
        bool itemVisible = anySubChildrenVisible || shouldItemBeShown( item );
        item->setVisible( itemVisible );
        anyChildrenVisible |= itemVisible;
    }
    return anyChildrenVisible;
}

AnnotationDialog::ListViewTextMatchHider::ListViewTextMatchHider( const QString& text, bool anchorAtStart, QListView* listView )
    :_text( text ), _anchorAtStart( anchorAtStart )
{
    setItemsVisible( listView );
}

bool AnnotationDialog::ListViewTextMatchHider::shouldItemBeShown( QListViewItem* item )
{
    if ( _anchorAtStart )
        return item->text(0).lower().startsWith( _text.lower() );
    else
        return item->text(0).lower().contains( _text.lower() );
}

bool AnnotationDialog::ListViewCheckedHider::shouldItemBeShown( QListViewItem* item )
{
    return static_cast<QCheckListItem*>(item)->isOn();
}

AnnotationDialog::ListViewCheckedHider::ListViewCheckedHider( QListView* listView )
{
    setItemsVisible( listView );
}
