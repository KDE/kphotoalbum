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
#ifndef DB_CATEGORYITEMS_H
#define DB_CATEGORYITEMS_H

#include <ksharedptr.h>
#include <qstring.h>
#include <qvaluelist.h>

namespace DB
{
class CategoryItem :public KShared
{
public:
    CategoryItem( const QString& name, bool isTop = false ) : _name( name ), _isTop( isTop ) {}
    ~CategoryItem();
    CategoryItem* clone() const;
    bool isDescendentOf( const QString& child, const QString& parent ) const;

protected:
    void print( int offset );
    bool hasChild( const QString& child );

public:
    QString _name;
    QValueList< CategoryItem* > _subcategories;
    bool _isTop;
};

}


#endif /* DB_CATEGORYITEMS_H */

