// SPDX-FileCopyrightText: 2003-2020 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2005, 2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2008, 2010 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2008 Henner Zeller <h.zeller@acm.org>
// SPDX-FileCopyrightText: 2008 Jan Kundrát <jkt@flaska.net>
// SPDX-FileCopyrightText: 2008 Laurent Montel <montel@kde.org>
// SPDX-FileCopyrightText: 2020 Robert Krawitz <rlk@alum.mit.edu>
// SPDX-FileCopyrightText: 2012-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2020-2024 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ImageDateCollection.h"
#include "ImageInfoList.h"

#include <utility>

DB::ImageDateCollection::ImageDateCollection(const ImageInfoList &list)
{
    for (const auto &image : list) {
        const auto &date = image->date();
        m_startIndex.insert(date.start(), date);
    }
    buildIndex();
}

void DB::ImageDateCollection::buildIndex()
{
    StartIndexMap::ConstIterator startSearch = m_startIndex.constBegin();
    Utilities::FastDateTime biggestEnd = QDate(1900, 1, 1).startOfDay();
    for (StartIndexMap::ConstIterator it = m_startIndex.constBegin();
         it != m_startIndex.constEnd();
         ++it) {
        // We want a monotonic mapping end-date -> smallest-in-start-index.
        // Since we go through the start index sorted, lowest first, we just
        // have to keep the last pointer as long as we find smaller end-dates.
        // This should be rare as it only occurs if there are images that
        // actually represent a range not just a point in time.
        if (it.value().end() > biggestEnd) {
            biggestEnd = it.value().end();
            startSearch = it;
        }
        m_endIndex.insert(it.value().end(), startSearch);
    }
}

/**
 * @brief DB::ImageDateCollection::count
 * @param range
 * @return
 * @internal
   Previously, counting the elements was done by going through all elements
   and count the matches for a particular range, this unfortunately had
   O(n) complexity multiplied by m ranges we would get O(mn).

   Henner Zeller rewrote it to its current state. The main idea now is to
   have all dates sorted so that it is possible to only look at the
   requested range. Since it is not points in time, we can't have just a
   simple sorted list. So we have two sorted maps, the m_startIndex and
   m_endIndex. m_startIndex is sorted by the start time of all ImageDates
   (which are in fact ranges)

   If we would just look for Images that start _after_ the query-range, we
   would miscount, because there might be Image ranges starting before the
   query time but whose end time reaches into the query range this is what
   the m_endIndex is for: it is sortd by end-date; here we look for
   everything that is >= our query start. its value() part is basically a
   pointer to the position in the m_startIndex where we actually have to
   start looking.

   The rest is simple: we determine the interesting start in
   m_startIndex using the m_endIndex and iterate through it until the
   elements in that sorted list have a start time that is larger than the
   query-end-range .. there will no more elements coming.

   The above uses the fact that a QMap::constIterator iterates the map in
   sorted order.
 * @endinternal
 */
DB::ImageCount DB::ImageDateCollection::count(const DB::ImageDate &range)
{
    if (m_cache.contains(range))
        return m_cache[range];

    int exact = 0, rangeMatch = 0;

    // We start searching in ranges that overlap our start search range, i.e.
    // where the end-date is higher than our search start.
    const EndIndexMap::ConstIterator endSearch = std::as_const(m_endIndex).lowerBound(range.start());

    if (endSearch != m_endIndex.constEnd()) {
        // qDebug() << "Counting images until" << endSearch.key() << "starting at" << endSearch.value().key();
        for (StartIndexMap::ConstIterator it = endSearch.value();
             it != m_startIndex.constEnd() && it.key() <= range.end();
             ++it) {
            using MatchType = DB::ImageDate::MatchType;
            MatchType tp = it.value().isIncludedIn(range);
            switch (tp) {
            case MatchType::IsContained:
                exact++;
                break;
            case MatchType::Overlap:
                rangeMatch++;
                break;
            case MatchType::NoMatch:
                break;
            }
        }
    }

    DB::ImageCount res(exact, rangeMatch);
    m_cache.insert(range, res); // TODO(hzeller) this might go now
    return res;
}

Utilities::FastDateTime DB::ImageDateCollection::lowerLimit() const
{
    if (!m_startIndex.empty()) {
        // skip null dates:
        for (StartIndexMap::ConstIterator it = m_startIndex.constBegin(); it != m_startIndex.constEnd(); ++it) {
            if (it.key().isValid())
                return it.key();
        }
    }
    // FIXME(jzarl): wouldn't it be better to return a null date instead?
    return QDate(1900, 1, 1).startOfDay();
}

Utilities::FastDateTime DB::ImageDateCollection::upperLimit() const
{
    if (!m_endIndex.empty()) {
        EndIndexMap::ConstIterator highest = m_endIndex.constEnd();
        --highest;
        return highest.key();
    }
    // FIXME(jzarl): wouldn't it be better to return a null date instead?
    return QDate(2100, 1, 1).startOfDay();
}

// vi:expandtab:tabstop=4 shiftwidth=4:
