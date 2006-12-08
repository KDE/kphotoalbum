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
#include "DragObject.h"
const char* CategoryListView::DragObject::format( int i ) const
{
    if ( i == 0 )
        return "x-kphotoalbum/x-category-drag";
    else
        return 0;
}

QByteArray CategoryListView::DragObject::encodedData( const char* ) const
{
    QByteArray res;
    QDataStream stream( res, IO_WriteOnly );
    stream << _items;
    return res;
}

CategoryListView::DragObject::DragObject( const CategoryListView::DragItemInfoSet& items, QWidget* dragSource )
    :QDragObject( dragSource ), _items( items )
{
}
