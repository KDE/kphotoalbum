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
#include "DatabaseElement.h"
#include <qsqlquery.h>
#include <exiv2/exif.hpp>

static QString replaceDotWithUnderscore( const char* cstr )
{
    QString str( QString::fromLatin1( cstr ) );
    return str.replace( QString::fromLatin1( "." ), QString::fromLatin1( "_" ) );
}

Exif::StringExifElement::StringExifElement( const char* tag )
    : _tag( tag )
{
}


QString Exif::StringExifElement::createString()
{
    return QString::fromLatin1( "%1 string" ).arg( replaceDotWithUnderscore( _tag ) );
}


QString Exif::StringExifElement::queryString()
{
    return QString::fromLatin1( "?" );
}


void Exif::StringExifElement::bindValues( QSqlQuery* query, int& counter, Exiv2::ExifData& data )
{
    query->bindValue( counter++, data[_tag].toString().c_str() );
}


Exif::IntExifElement::IntExifElement( const char* tag )
 : _tag( tag )
{
}


QString Exif::IntExifElement::createString()
{
    return QString::fromLatin1( "%1 int" ).arg( replaceDotWithUnderscore( _tag ) );
}


QString Exif::IntExifElement::queryString()
{
    return QString::fromLatin1( "?" );
}


void Exif::IntExifElement::bindValues( QSqlQuery* query, int& counter, Exiv2::ExifData& data )
{
    query->bindValue( counter++, (int) data[_tag].toLong() );
}


Exif::RationalExifElement::RationalExifElement( const char* tag )
 : _tag( tag )
{
}


QString Exif::RationalExifElement::createString()
{
    return QString::fromLatin1( "%1 float" ).arg( replaceDotWithUnderscore( _tag ) );
}

QString Exif::RationalExifElement::queryString()
{
    return QString::fromLatin1( "?" );
}


void Exif::RationalExifElement::bindValues( QSqlQuery* query, int& counter, Exiv2::ExifData& data )
{
    query->bindValue( counter++, 1.0 * data[_tag].toRational().first / data[_tag].toRational().second);
}

