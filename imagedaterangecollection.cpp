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

#include "imagedaterangecollection.h"
#include "imageinfo.h"

ImageDateRangeCollection::ImageDateRangeCollection()
    : _dirtyLower( false ), _dirtyUpper( false )
{
}

void ImageDateRangeCollection::append( const ImageDateRange& dateRange )
{
    _dates.append( dateRange );
    _dirtyLower = true;
    _dirtyUpper = true;
}

ImageCount ImageDateRangeCollection::count( const ImageDate& from, const ImageDate& to )
{
    ImageDateRange range( from, to );
    if ( _cache.contains( range ) )
        return _cache[range];

    int exact = 0, rangeMatch = 0;
    for( QValueList<ImageDateRange>::Iterator it = _dates.begin(); it != _dates.end(); ++it ) {
        ImageDateRange::MatchType tp = (*it).isIncludedIn( ImageDateRange( from, to ) );
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

QDateTime ImageDateRangeCollection::lowerLimit() const
{
    static QDateTime _lower = QDateTime( QDate( 1900, 1, 1 ) );
    if ( _dirtyLower && _dates.count() != 0 ) {
        bool first = true;
        for( QValueList<ImageDateRange>::ConstIterator it = _dates.begin(); it != _dates.end(); ++it ) {
            ImageDate date = (*it).start();
            if ( date.year() == 0 )
                continue;

            if ( first ) {
                _lower = date.min();
                first = false;
            }
            else if ( date.min() < _lower )
                _lower = date.min();
        }
    }
    _dirtyLower = false;
    return _lower;
}

QDateTime ImageDateRangeCollection::upperLimit() const
{
    static QDateTime _upper = QDateTime( QDate( 2100, 1, 1 ) );
    if ( _dirtyUpper && _dates.count() != 0 ) {
        bool first = true;
        for( QValueList<ImageDateRange>::ConstIterator it = _dates.begin(); it != _dates.end(); ++it ) {
            ImageDate date = (*it).end();
            if ( date.year() == 0 )
                date = (*it).start();
            if ( date.year() == 0 )
                continue;

            if ( first ) {
                _upper = date.max();
                first = false;
            }
            else if ( date.max() > _upper ) {
                _upper = date.max();
            }
        }
    }
    _dirtyUpper = false;
    return _upper;
}

ImageDateRangeCollection::ImageDateRangeCollection( const ImageInfoList& list )
{
    for( ImageInfoListIterator it( list ); *it; ++it ) {
        append( ImageDateRange( (*it)->startDate(), (*it)->endDate() ) );
    }
}

