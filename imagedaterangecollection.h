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

#ifndef IMAGEDATERANGECOLLECTION_H
#define IMAGEDATERANGECOLLECTION_H
#include "imagedaterange.h"
#include <qvaluelist.h>
#include <qmap.h>
#include "imageinfolist.h"

class ImageCount
{
public:
    ImageCount( int exact, int rangeMatch )
        : _exact( exact ), _rangeMatch( rangeMatch )
        {
        }
    ImageCount() {}

    int _exact;
    int _rangeMatch;
};


class ImageDateRangeCollection
{
public:
    ImageDateRangeCollection();
    ImageDateRangeCollection( const ImageInfoList& );
    void append( const ImageDateRange& );
    ImageCount count( const ImageDate& from, const ImageDate& to );
    QDateTime lowerLimit() const;
    QDateTime upperLimit() const;

private:
    QValueList<ImageDateRange> _dates;
    QMap<ImageDateRange,ImageCount> _cache;
    mutable bool _dirtyLower, _dirtyUpper;
};


#endif /* IMAGEDATERANGECOLLECTION_H */

