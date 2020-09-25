/* Copyright (C) 2003-2020 The KPhotoAlbum Development Team

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
#ifndef XMLIMAGEDATECOLLECTION_H
#define XMLIMAGEDATECOLLECTION_H

#include <DB/ImageDateCollection.h>

#include <QMap>

namespace DB
{
class ImageInfoList;
}

namespace XMLDB
{
class XMLImageDateCollection : public DB::ImageDateCollection
{
public:
    explicit XMLImageDateCollection(const DB::ImageInfoList &);

public:
    DB::ImageCount count(const DB::ImageDate &range) override;
    Utilities::FastDateTime lowerLimit() const override;
    Utilities::FastDateTime upperLimit() const override;

private:
    typedef QMap<Utilities::FastDateTime, DB::ImageDate> StartIndexMap;
    typedef QMap<Utilities::FastDateTime, StartIndexMap::ConstIterator> EndIndexMap;

    void add(const DB::ImageDate &);

    // Build index, after all elements have been added.
    void buildIndex();

    // Cache for past successful range lookups.
    QMap<DB::ImageDate, DB::ImageCount> m_cache;

    // Elements ordered by start time.
    //
    // Start index is sorted by start time of the ImageDate, mapping
    // to the actual ImageDate; this is a multimap.
    StartIndexMap m_startIndex;

    // Pointers to start index ordered by end time.
    //
    // This maps the end date to an iterator into the startIndex. The
    // iterator points to the lowest element in startIndex whose end-time
    // is greater or equal to the key-time. Thus is points to the start
    // where its worth looking.
    EndIndexMap m_endIndex;
};
}

#endif /* XMLIMAGEDATECOLLECTION_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
