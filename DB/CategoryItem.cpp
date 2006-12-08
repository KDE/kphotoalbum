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
#include "CategoryItem.h"
DB::CategoryItem::~CategoryItem()
{
    for( QValueList<CategoryItem*>::ConstIterator it = _subcategories.begin(); it != _subcategories.end(); ++it ) {
        delete *it;
    }
}

DB::CategoryItem* DB::CategoryItem::clone() const
{
    CategoryItem* result = new CategoryItem( _name );
    for( QValueList<CategoryItem*>::ConstIterator it = _subcategories.begin(); it != _subcategories.end(); ++it ) {
        result->_subcategories.append( (*it)->clone() );
    }
    return result;
}

void DB::CategoryItem::print( int offset )
{
    QString spaces;
    spaces.fill( ' ', offset );
    qDebug( "%s%s", spaces.latin1(), _name.latin1() );
    for( QValueList< CategoryItem* >::Iterator it = _subcategories.begin(); it != _subcategories.end(); ++it ) {
        (*it)->print( offset + 2 );
    }
}

bool DB::CategoryItem::isDescendentOf( const QString& child, const QString& parent ) const
{
   for( QValueList< CategoryItem* >::ConstIterator it = _subcategories.begin(); it != _subcategories.end(); ++it ) {
        if ( _name == parent ) {
            if ( (*it)->hasChild( child ) )
                return true;
        }
        else {
            if ( (*it)->isDescendentOf( child, parent ) )
                return true;
        }
    }
    return false;
}

bool DB::CategoryItem::hasChild( const QString& child )
{
    if ( _name == child )
        return true;

    for( QValueList< CategoryItem* >::Iterator it = _subcategories.begin(); it != _subcategories.end(); ++it ) {
        if ( (*it)->hasChild( child ) )
            return true;
    }
    return false;
}



