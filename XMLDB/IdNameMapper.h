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
#ifndef IDNAMEMAPPER_H
#define IDNAMEMAPPER_H

#include "DB/RawId.h"
#include <QMap>
#include <QString>
#include <DB/FileName.h>

namespace DB
{
class IdNameMapper
{
public:
    IdNameMapper();
    void add( const DB::FileName& fileName );
    void remove( DB::RawId id );
    void remove( const DB::FileName& fileName );

    bool exists(const DB::FileName & filename) const;

    DB::RawId operator[](const DB::FileName& ) const;
    DB::FileName operator[]( DB::RawId ) const;
private:
    QMap<DB::RawId, DB::FileName> _idTofileName;
    QMap<DB::FileName, DB::RawId> _fileNameToId;
    int _maxId;
};

}


#endif /* IDNAMEMAPPER_H */

