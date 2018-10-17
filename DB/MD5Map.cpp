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

#include "MD5Map.h"

using namespace DB;

void MD5Map::insert( const MD5& md5sum, const DB::FileName& fileName )
{
    m_map.insert( md5sum, fileName );
    m_i_map.insert( fileName, md5sum );
}

DB::FileName MD5Map::lookup( const MD5& md5sum ) const
{
    return m_map[md5sum];
}

MD5 MD5Map::lookupFile( const DB::FileName& fileName ) const
{
    return m_i_map[fileName];
}

bool MD5Map::contains( const MD5& md5sum ) const
{
    return m_map.contains( md5sum );
}

bool MD5Map::containsFile( const DB::FileName& fileName ) const
{
    return m_i_map.contains( fileName );
}

void MD5Map::clear()
{
    m_map.clear();
    m_i_map.clear();
}

DB::FileNameSet DB::MD5Map::diff( const MD5Map& other ) const
{
    DB::FileNameSet res;

    for( MD5FileMap::ConstIterator it = m_map.begin(); it != m_map.end(); ++it ) {
        if ( other.lookup( it.key() ) != it.value() )
            res.insert( it.value() );
    }

    return res;
}
// vi:expandtab:tabstop=4 shiftwidth=4:
