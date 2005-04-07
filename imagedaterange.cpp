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

#include "imagedaterange.h"
ImageDateRange::ImageDateRange( const ImageDate& from, const ImageDate& to )
{
    if ( from <= to ) {
        _imageStart = from;
        _imageEnd = to;
    }
    else {
        _imageStart = to;
        _imageEnd = from;
    }
}

ImageDateRange::ImageDateRange()
{
}

ImageDateRange::MatchType ImageDateRange::isIncludedIn( const ImageDateRange& searchRange )
{
    // We don't really want to include images without a year, as they clutter up things too much.
    if ( _imageStart.year() == 0  || ( !_imageEnd.isNull() && _imageEnd.year() == 0 ) )
        return DontMatch;

    ImageDate searchStart = searchRange.start();
    ImageDate searchEnd = searchRange.end();

    // We always want a range, so if no range is specified, then copy from-date to to-date.
    ImageDate imageEnd( _imageEnd );
    if ( _imageEnd.isNull() )
        imageEnd = _imageStart;
    if ( searchEnd.isNull() )
        searchEnd = searchStart;

    if ( searchStart.min() <= _imageStart.min() && searchEnd.max() >= imageEnd.max() )
        return ExactMatch;

    if ( searchStart.min() <= imageEnd.max() && searchEnd.max() >= _imageStart.min() ) {
        return RangeMatch;
    }
    return DontMatch;
}

bool ImageDateRange::includes( const QDateTime& date )
{
    return ImageDateRange( ImageDate( date ), ImageDate(date) ).isIncludedIn( *this ) == ExactMatch;
}


ImageDate ImageDateRange::start() const
{
    return _imageStart;
}

ImageDate ImageDateRange::end() const
{
    return _imageEnd;
}

bool ImageDateRange::operator<(const ImageDateRange& other ) const
{
    return _imageStart < other._imageStart ||
        ( _imageStart == other._imageStart && _imageEnd < other._imageEnd );
}

