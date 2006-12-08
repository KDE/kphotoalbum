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
#ifndef DATABASEELEMENT_H
#define DATABASEELEMENT_H

#include <qstring.h>
namespace Exiv2
{
    class ExifData;
}
class QSqlQuery;

namespace Exif {

class DatabaseElement
{
public:
    virtual QString createString() = 0; // Exif_Photo_FNumber_denominator int, Exif_Photo_FNumber_nominator int
    virtual QString queryString() = 0; // ?, ?
    virtual void bindValues( QSqlQuery*, int& counter, Exiv2::ExifData& data ) = 0; // bind values
};

class StringExifElement :public DatabaseElement
{
public:
    StringExifElement( const char* tag );
    QString createString();
    QString queryString();
    void bindValues( QSqlQuery* query, int& counter, Exiv2::ExifData& data );

private:
    const char* _tag;
};

class IntExifElement :public DatabaseElement
{
public:
    IntExifElement( const char* tag );
    QString createString();
    QString queryString();
    void bindValues( QSqlQuery* query, int& counter, Exiv2::ExifData& data );

private:
    const char* _tag;
};


class RationalExifElement :public DatabaseElement
{
public:
    RationalExifElement( const char* tag );
    virtual QString createString();
    virtual QString queryString();
    virtual void bindValues( QSqlQuery* query, int& counter, Exiv2::ExifData& data );

private:
    const char* _tag;
};


}



#endif /* DATABASEELEMENT_H */

