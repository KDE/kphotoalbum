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
#include "IdNameMapper.h"

void DB::IdNameMapper::add( const QString& fileName )
{
    const DB::RawId id(++_maxId);
    Q_ASSERT(id != DB::RawId());
    _idTofileName.insert( id, fileName );
    _fileNameToId.insert( fileName, id );
    Q_ASSERT( !fileName.startsWith(QLatin1String("/")));
}

DB::RawId DB::IdNameMapper::operator[](const QString& fileName ) const
{
    Q_ASSERT( !fileName.startsWith( QLatin1String("/") ) );
    if ( !_fileNameToId.contains( fileName ) )
        return DB::RawId();
    return _fileNameToId[fileName];
}

QString DB::IdNameMapper::operator[]( DB::RawId id ) const
{
    if (!_idTofileName.contains( id ) ) {
        return QString::fromLatin1( "" );
    }
    return _idTofileName[id];
}

void DB::IdNameMapper::remove( DB::RawId id )
{
    if( !_idTofileName.contains( id ) )
        return;
;
    _fileNameToId.remove( _idTofileName[id] );
    _idTofileName.remove( id );
}

void DB::IdNameMapper::remove( const QString& fileName )
{
    if ( !_fileNameToId.contains( fileName ) )
        return;
    Q_ASSERT( !fileName.startsWith( QLatin1String("/") ) );
    _idTofileName.remove( _fileNameToId[fileName] );
    _fileNameToId.remove( fileName );
}

DB::IdNameMapper::IdNameMapper()
    :_maxId(0)
{
}

bool DB::IdNameMapper::exists(const QString& fileName ) const
{
  return _fileNameToId.find(fileName) != _fileNameToId.end();
}
