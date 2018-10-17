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

#ifndef MD5MAP_H
#define MD5MAP_H
#include <qstring.h>
#include <qhash.h>
#include "MD5.h"
#include <DB/FileName.h>

namespace DB
{
    typedef QHash<MD5, DB::FileName> MD5FileMap;
    typedef QHash<DB::FileName, MD5> FileMD5Map;

/**
   This class may be overridden by a which wants to store md5 information
   directly in a database, rather than in a map in memory.
**/
class MD5Map
{
public:
    virtual ~MD5Map() {}
    virtual void insert( const MD5& md5sum, const DB::FileName& fileName );
    virtual DB::FileName lookup( const MD5& md5sum ) const;
    virtual MD5 lookupFile( const DB::FileName& fileName ) const;
    virtual bool contains( const MD5& md5sum ) const;
    virtual bool containsFile( const DB::FileName& fileName ) const;
    virtual void clear();
    virtual DB::FileNameSet diff( const MD5Map& other ) const;

private:
    MD5FileMap m_map;
    FileMD5Map m_i_map;
};

}


#endif /* MD5MAP_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
