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

#include "XMLImageDateCollection.h"
#include "imageinfo.h"
#include "imagedb.h"

XMLImageDateCollection::XMLImageDateCollection()
    : _dirtyLower( false ), _dirtyUpper( false )
{
}

void XMLImageDateCollection::append( const ImageDate& date )
{
    _dates.append( date );
    _dirtyLower = true;
    _dirtyUpper = true;
}

ImageCount XMLImageDateCollection::count( const ImageDate& range )
{
    if ( _cache.contains( range ) )
        return _cache[range];

    int exact = 0, rangeMatch = 0;
    for( QValueList<ImageDate>::Iterator it = _dates.begin(); it != _dates.end(); ++it ) {
        ImageDate::MatchType tp = (*it).isIncludedIn( range );
        switch (tp) {
        case ImageDate::ExactMatch: exact++;break;
        case ImageDate::RangeMatch: rangeMatch++; break;
        case ImageDate::DontMatch: break;
        }
    }

    ImageCount res( exact, rangeMatch );
    _cache.insert( range, res );
    return res;
}

QDateTime XMLImageDateCollection::lowerLimit() const
{
    static QDateTime _lower = QDateTime( QDate( 1900, 1, 1 ) );
    if ( _dirtyLower && _dates.count() != 0 ) {
        bool first = true;
        for( QValueList<ImageDate>::ConstIterator it = _dates.begin(); it != _dates.end(); ++it ) {
            if ( first ) {
                _lower = (*it).start();
                first = false;
            }
            else if ( (*it).start() < _lower )
                _lower = (*it).start();
        }
    }
    _dirtyLower = false;
    return _lower;
}

QDateTime XMLImageDateCollection::upperLimit() const
{
    static QDateTime _upper = QDateTime( QDate( 2100, 1, 1 ) );
    if ( _dirtyUpper && _dates.count() != 0 ) {
        bool first = true;
        for( QValueList<ImageDate>::ConstIterator it = _dates.begin(); it != _dates.end(); ++it ) {
            if ( first ) {
                _upper = (*it).end();
                first = false;
            }
            else if ( (*it).end() > _upper ) {
                _upper = (*it).end();
            }
        }
    }
    _dirtyUpper = false;
    return _upper;
}

XMLImageDateCollection::XMLImageDateCollection( const QStringList& list )
{
    for( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
        ImageInfoPtr info = ImageDB::instance()->info( *it );
        append( info->date() );
    }
}

