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
#include "DragableListView.h"
#include "DragObject.h"
CategoryListView::DragableListView::DragableListView( const DB::CategoryPtr& category, QWidget* parent, const char* name )
    :QListView( parent, name ), _category( category )
{
}

QDragObject* CategoryListView::DragableListView::dragObject()
{
    CategoryListView::DragItemInfoSet selected;
    for ( QListViewItemIterator itemIt( this ); *itemIt; ++itemIt ) {
        if ( (*itemIt)->isSelected() ) {
            QListViewItem* parent = (*itemIt)->parent();
            QString parentText = parent ? parent->text(0) : QString::null;
            selected.insert( CategoryListView::DragItemInfo( parentText, (*itemIt)->text(0) ) );
        }
    }

    return new DragObject( selected, this );
}

DB::CategoryPtr CategoryListView::DragableListView::category() const
{
    return _category;
}

void CategoryListView::DragableListView::emitItemsChanged()
{
    emit itemsChanged();
}

#include "DragableListView.moc"
