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
#ifndef XMLIMAGEDATERANGECOLLECTION_H
#define XMLIMAGEDATERANGECOLLECTION_H
#include "DB/ImageDateCollection.h"

class XMLImageDateCollection :public DB::ImageDateCollection
{
public:
    XMLImageDateCollection();
    XMLImageDateCollection( const QStringList& );

public:
    virtual DB::ImageCount count( const DB::ImageDate& range );
    virtual QDateTime lowerLimit() const;
    virtual QDateTime upperLimit() const;

private:
    void append( const DB::ImageDate& );

    QValueList<DB::ImageDate> _dates;
    QMap<DB::ImageDate,DB::ImageCount> _cache;
    mutable bool _dirtyLower, _dirtyUpper;
};


#endif /* XMLIMAGEDATERANGECOLLECTION_H */

