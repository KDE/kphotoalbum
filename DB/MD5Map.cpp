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

#include "MD5Map.h"

using namespace DB;

void MD5Map::insert( const MD5& md5sum, const QString& fileName )
{
    _map.insert( md5sum, fileName );
}

QString MD5Map::lookup( const MD5& md5sum ) const
{
    return _map[md5sum];
}

bool MD5Map::contains( const MD5& md5sum ) const
{
    return _map.contains( md5sum );
}

void MD5Map::clear()
{
    _map.clear();
}

StringSet DB::MD5Map::diff( const MD5Map& other ) const
{
    StringSet res;

    for( QMap<MD5, QString>::ConstIterator it = _map.begin(); it != _map.end(); ++it ) {
        if ( other.lookup( it.key() ) != it.data() )
            res.insert( it.data() );
    }

    return res;
}
