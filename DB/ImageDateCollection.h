// SPDX-FileCopyrightText: 2003 - 2012 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2005, 2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2007 Laurent Montel <montel@kde.org>
// SPDX-FileCopyrightText: 2008 Henner Zeller <h.zeller@acm.org>
// SPDX-FileCopyrightText: 2008, 2010 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2013 Dominik Broj <broj.dominik@gmail.com>
// SPDX-FileCopyrightText: 2020 Robert Krawitz <rlk@alum.mit.edu>
// SPDX-FileCopyrightText: 2013 - 2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IMAGEDATECOLLECTION_H
#define IMAGEDATECOLLECTION_H
#include "ImageDate.h"

#include <QExplicitlySharedDataPointer>
#include <qmap.h>
namespace DB
{
class ImageInfoList;

/**
 * @brief The ImageCount struct represents a count of exact and range matches.
 */
struct ImageCount {
    ImageCount(int exact, int rangeMatch)
        : mp_exact(exact)
        , mp_rangeMatch(rangeMatch)
    {
    }
    ImageCount()
        : mp_exact(0)
        , mp_rangeMatch(0)
    {
    }

    /**
     * @brief mp_exact counts images which were contained in the queried date range.
     */
    int mp_exact;
    /**
     * @brief mp_rangeMatch counts images which have a fuzzy date which overlaps with the queried date range.
     */
    int mp_rangeMatch;
};

/**
 * @brief The ImageDateCollection class implements an efficient way to the images in a given set that fall withing a given date range.
 * The class is optimized for repeated queries on the same set of images but for a different date range (as is required for building a histogram like the one in the DateBar).
 *
 */
class ImageDateCollection : public QSharedData
{
public:
    /**
     * @brief ImageDateCollection constructs an ImageDateCollection for the given image list.
     * @param list
     */
    explicit ImageDateCollection(const DB::ImageInfoList &list);
    /**
     * @brief count all images in the collection that match the given ImageDate.
     * Usually, the ImageDate is a fuzzy date defining a date range.
     * @param range
     * @return a count of exact and range matches
     */
    ImageCount count(const ImageDate &range);
    /**
     * @brief lowerLimit
     * @return the date/tme of the earliest starting image in the ImageDateCollection, ignoring null dates.
     */
    Utilities::FastDateTime lowerLimit() const;
    /**
     * @brief upperLimit
     * @return the date/time of the latest ending image in the  ImageDateCollection, ignoring null dates
     */
    Utilities::FastDateTime upperLimit() const;

private:
    typedef QMultiMap<Utilities::FastDateTime, DB::ImageDate> StartIndexMap;
    typedef QMap<Utilities::FastDateTime, StartIndexMap::ConstIterator> EndIndexMap;

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

#endif /* IMAGEDATECOLLECTION_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
