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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifdef HASKIPI
#include "imageinfo.h"
#include "categoryimagecollection.h"
#include <klocale.h>
#include "imagedb.h"
CategoryImageCollection::CategoryImageCollection( const ImageSearchInfo& context, const QString& category,
                                                  const QString& value )
    : MyImageCollection( CategoryImageCollection::SubClass ), _context( context ), _category( category ),
      _value( value )
{
}

QString CategoryImageCollection::name()
{
    if ( _value == QString::fromLatin1( "**NONE**" ) )
        return i18n( "Example: No Persons", "No %1" ).arg( _category );
    else
        return _value;
}

KURL::List CategoryImageCollection::images()
{
    ImageSearchInfo context( _context );
    context.addAnd( _category, _value );
    ImageInfoList list = ImageDB::instance()->images( context, true );
    return imageListToUrlList( list );
}
#endif // KIPI
