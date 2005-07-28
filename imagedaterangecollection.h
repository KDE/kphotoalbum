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

#ifndef IMAGEDATERANGECOLLECTION_H
#define IMAGEDATERANGECOLLECTION_H
#include "imagedaterange.h"
#include <qvaluelist.h>
#include <qmap.h>
#include "imageinfolist.h"
#include <ksharedptr.h>
class QStringList;

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


class ImageDateRangeCollection :public KShared
{
public:
    virtual ImageCount count( const ImageDateRange& range ) = 0;
    virtual QDateTime lowerLimit() const = 0;
    virtual QDateTime upperLimit() const = 0;
};


#endif /* IMAGEDATERANGECOLLECTION_H */

