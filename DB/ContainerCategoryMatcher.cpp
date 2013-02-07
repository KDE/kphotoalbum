/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
#include "ContainerCategoryMatcher.h"

void DB::ContainerCategoryMatcher::addElement( CategoryMatcher* element )
{
    _elements.append( element );
}

DB::ContainerCategoryMatcher::~ContainerCategoryMatcher()
{
    for( int i = 0; i < _elements.count(); ++i )
        delete _elements[i];
}

void DB::ContainerCategoryMatcher::debug( int level ) const
{
     for( QList<CategoryMatcher*>::ConstIterator it = _elements.begin(); it != _elements.end(); ++it ) {
        (*it)->debug( level );
    }
}


void DB::ContainerCategoryMatcher::setShouldCreateMatchedSet(bool b)
{
    _shouldPrepareMatchedSet = b;
    Q_FOREACH( DB::CategoryMatcher* matcher,_elements )
        matcher->setShouldCreateMatchedSet( b );
}

// vi:expandtab:tabstop=4 shiftwidth=4:
