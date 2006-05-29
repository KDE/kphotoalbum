/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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

#include <config.h>
#ifdef HASKIPI
#include "DB/ImageInfo.h"
#include "Plugins/CategoryImageCollection.h"
#include <klocale.h>
#include "DB/ImageDB.h"
Plugins::CategoryImageCollection::CategoryImageCollection( const DB::ImageSearchInfo& context, const QString& category,
                                                  const QString& value )
    : Plugins::ImageCollection( CategoryImageCollection::SubClass ), _context( context ), _category( category ),
      _value( value )
{
}

QString Plugins::CategoryImageCollection::name()
{
    if ( _value == QString::fromLatin1( "**NONE**" ) )
        return i18n( "Example: No Persons", "No %1" ).arg( _category );
    else
        return _value;
}

KURL::List Plugins::CategoryImageCollection::images()
{
    DB::ImageSearchInfo context( _context );
    context.addAnd( _category, _value );
    QStringList list = DB::ImageDB::instance()->search( context, true );
    return stringListToUrlList( list );
}
#endif // KIPI
