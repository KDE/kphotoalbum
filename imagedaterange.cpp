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

#include "imagedaterange.h"
ImageDateRange::ImageDateRange( const ImageDate& date )
{
    _date = date;
}

ImageDateRange::ImageDateRange()
{
}

ImageDateRange::MatchType ImageDateRange::isIncludedIn( const ImageDateRange& searchRange )
{
    if ( searchRange.date().start() <= _date.start() && searchRange.date().end() >= _date.end() )
        return ExactMatch;

    if ( searchRange.date().start() <= _date.end() && searchRange.date().end() >= _date.start() ) {
        return RangeMatch;
    }
    return DontMatch;
}

bool ImageDateRange::includes( const QDateTime& date )
{
    return ImageDateRange( ImageDate( date ) ).isIncludedIn( *this ) == ExactMatch;
}


bool ImageDateRange::operator<(const ImageDateRange& other ) const
{
    return _date < other._date;
}

ImageDate ImageDateRange::date() const
{
    return _date;
}

