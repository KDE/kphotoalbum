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

#include "XMLImageDateRangeCollection.h"
#include "imageinfo.h"
#include "imagedb.h"

XMLImageDateRangeCollection::XMLImageDateRangeCollection()
    : _dirtyLower( false ), _dirtyUpper( false )
{
}

void XMLImageDateRangeCollection::append( const ImageDateRange& dateRange )
{
    _dates.append( dateRange );
    _dirtyLower = true;
    _dirtyUpper = true;
}

ImageCount XMLImageDateRangeCollection::count( const ImageDateRange& range )
{
    if ( _cache.contains( range ) )
        return _cache[range];

    int exact = 0, rangeMatch = 0;
    for( QValueList<ImageDateRange>::Iterator it = _dates.begin(); it != _dates.end(); ++it ) {
        ImageDateRange::MatchType tp = (*it).isIncludedIn( range );
        switch (tp) {
        case ImageDateRange::ExactMatch: exact++;break;
        case ImageDateRange::RangeMatch: rangeMatch++; break;
        case ImageDateRange::DontMatch: break;
        }
    }

    ImageCount res( exact, rangeMatch );
    _cache.insert( range, res );
    return res;
}

QDateTime XMLImageDateRangeCollection::lowerLimit() const
{
    static QDateTime _lower = QDateTime( QDate( 1900, 1, 1 ) );
    if ( _dirtyLower && _dates.count() != 0 ) {
        bool first = true;
        for( QValueList<ImageDateRange>::ConstIterator it = _dates.begin(); it != _dates.end(); ++it ) {
            ImageDate date = (*it).date();

            if ( first ) {
                _lower = date.start();
                first = false;
            }
            else if ( date.start() < _lower )
                _lower = date.start();
        }
    }
    _dirtyLower = false;
    return _lower;
}

QDateTime XMLImageDateRangeCollection::upperLimit() const
{
    static QDateTime _upper = QDateTime( QDate( 2100, 1, 1 ) );
    if ( _dirtyUpper && _dates.count() != 0 ) {
        bool first = true;
        for( QValueList<ImageDateRange>::ConstIterator it = _dates.begin(); it != _dates.end(); ++it ) {
            ImageDate date = (*it).date();

            if ( first ) {
                _upper = date.end();
                first = false;
            }
            else if ( date.end() > _upper ) {
                _upper = date.end();
            }
        }
    }
    _dirtyUpper = false;
    return _upper;
}

XMLImageDateRangeCollection::XMLImageDateRangeCollection( const QStringList& list )
{
    for( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
        ImageInfoPtr info = ImageDB::instance()->info( *it );
        append( ImageDateRange( info->date() ) );
    }
}

