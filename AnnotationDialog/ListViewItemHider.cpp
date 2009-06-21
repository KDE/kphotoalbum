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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "ListViewItemHider.h"

/**
 * \class AnnotationDialog::ListViewItemHider
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

void AnnotationDialog::ListViewItemHider::setItemsVisible( Q3ListView* listView )
{
    // It seems like a bug in Qt, but I need to make all item visible first, otherwise I see wrong items when I have items
    // two layers deep (A->B->c). It only occours when I widen (types "je", now deletes the "e" to get to "j" ).
    for ( Q3ListViewItemIterator it( listView ); *it; ++it )
        (*it)->setVisible( true );

    for ( Q3ListViewItem* item = listView->firstChild(); item; item = item->nextSibling() ) {
        bool anyChildrenVisible = setItemsVisible( item );
        bool visible = anyChildrenVisible || shouldItemBeShown( item );
        item->setVisible( visible );
    }
}

bool AnnotationDialog::ListViewItemHider::setItemsVisible( Q3ListViewItem* parentItem )
{
    bool anyChildrenVisible = false;
    for ( Q3ListViewItem* item = parentItem->firstChild(); item; item = item->nextSibling() ) {
        bool anySubChildrenVisible = setItemsVisible( item );
        bool itemVisible = anySubChildrenVisible || shouldItemBeShown( item );
        item->setVisible( itemVisible );
        anyChildrenVisible |= itemVisible;
    }
    return anyChildrenVisible;
}

AnnotationDialog::ListViewTextMatchHider::ListViewTextMatchHider( const QString& text, bool anchorAtStart, Q3ListView* listView )
    :_text( text ), _anchorAtStart( anchorAtStart )
{
    setItemsVisible( listView );
}

bool AnnotationDialog::ListViewTextMatchHider::shouldItemBeShown( Q3ListViewItem* item )
{
    if ( _anchorAtStart )
        return item->text(0).toLower().startsWith( _text.toLower() );
    else
        return item->text(0).toLower().contains( _text.toLower() );
}

bool AnnotationDialog::ListViewCheckedHider::shouldItemBeShown( Q3ListViewItem* item )
{
    return static_cast<Q3CheckListItem*>(item)->state() != Q3CheckListItem::Off;
}

AnnotationDialog::ListViewCheckedHider::ListViewCheckedHider( Q3ListView* listView )
{
    setItemsVisible( listView );
}
