/* SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
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
