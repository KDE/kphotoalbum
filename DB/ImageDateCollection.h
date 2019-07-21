/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef IMAGEDATECOLLECTION_H
#define IMAGEDATECOLLECTION_H
#include "ImageDate.h"

#include <QExplicitlySharedDataPointer>
#include <qmap.h>
namespace DB
{

class ImageCount
{
public:
    ImageCount(int exact, int rangeMatch)
        : mp_exact(exact)
        , mp_rangeMatch(rangeMatch)
    {
    }
    ImageCount() {}

    int mp_exact;
    int mp_rangeMatch;
};

class ImageDateCollection : public QSharedData
{
public:
    virtual ~ImageDateCollection() {}
    virtual ImageCount count(const ImageDate &range) = 0;
    virtual QDateTime lowerLimit() const = 0;
    virtual QDateTime upperLimit() const = 0;
};

}

#endif /* IMAGEDATECOLLECTION_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
