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
#include "SQLImageDateCollection.h"
#include <qvariant.h>
#include <qmap.h>
#include "QueryHelper.h"

using namespace SQLDB;

SQLImageDateCollection::SQLImageDateCollection(QueryHelper& queryHelper):
    _qh(queryHelper)
{
}

DB::ImageCount SQLImageDateCollection::count( const DB::ImageDate& range )
{
    // In a perfect world, we should check that the db hasn't changed, but
    // as we will get a new instance of this class each time the search
    // changes, it is really not that important, esp. because it is only
    // for the datebar, where a bit out of sync doesn't matter too much.

    static QMap<DB::ImageDate, DB::ImageCount> cache;
    if ( cache.contains( range ) )
        return cache[range];

    int exact =
        _qh.executeQuery(
            "SELECT COUNT(*) FROM file "
            "WHERE ?<=time_start AND time_end<=?",
            QueryHelper::Bindings() << range.start() << range.end()
            ).firstItem().toInt();
    int rng =
        _qh.executeQuery(
            "SELECT COUNT(*) FROM file "
            "WHERE ?<=time_end AND time_start<=?",
            QueryHelper::Bindings() << range.start() << range.end()
            ).firstItem().toInt() - exact;
    DB::ImageCount result( exact, rng );
    cache.insert( range, result );
    return result;
}

QDateTime SQLImageDateCollection::lowerLimit() const
{
    static QDateTime cachedLower;
    if (cachedLower.isNull())
        cachedLower = _qh.executeQuery(
            "SELECT MIN(time_start) FROM file"
            ).firstItem().toDateTime();
    return cachedLower;
}

QDateTime SQLImageDateCollection::upperLimit() const
{
    static QDateTime cachedUpper;
    if (cachedUpper.isNull())
        cachedUpper = _qh.executeQuery(
            "SELECT MAX(time_end) FROM file"
            ).firstItem().toDateTime();
    return cachedUpper;
}
