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

#ifndef MD5MAP_H
#define MD5MAP_H
#include <qstring.h>
#include <qmap.h>
#include "Utilities/Set.h"
#include "MD5.h"

namespace DB
{
using Utilities::StringSet;

/**
   This class may be overriden by a which wants to store md5 information
   directly in a database, rather than in a map in memory.
**/
class MD5Map
{
public:
    virtual void insert( const MD5& md5sum, const QString& fileName );
    virtual QString lookup( const MD5& md5sum ) const;
    virtual bool contains( const MD5& md5sum ) const;
    virtual void clear();
    virtual StringSet diff( const MD5Map& other ) const;

private:
    QMap<MD5, QString> _map;
};

}


#endif /* MD5MAP_H */

