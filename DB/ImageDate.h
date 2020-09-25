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

#ifndef IMAGEDATE_H
#define IMAGEDATE_H
#include <Utilities/FastDateTime.h>
#include <qstring.h>
#include <qstringlist.h>

namespace DB
{

class ImageDate
{
public:
    ImageDate();
    ImageDate(const QDate &start, const QDate &end);
    ImageDate(const Utilities::FastDateTime &start, const Utilities::FastDateTime &end);
    ImageDate(int yearFrom, int monthFrom, int dayFrom, int yearTo, int monthTo, int dayTo, int hourFrom, int minuteFrom, int secondFrom);
    explicit ImageDate(const QDate &);
    explicit ImageDate(const Utilities::FastDateTime &);
    ImageDate(const QDate &start, QDate end, const QTime &time);

    Utilities::FastDateTime start() const;
    Utilities::FastDateTime end() const;
    static QDate parseDate(const QString &date, bool startDate);

    bool operator<(const ImageDate &other) const;
    bool operator<=(const ImageDate &other) const;
    bool operator==(const ImageDate &other) const;
    bool operator!=(const ImageDate &other) const;

    bool isValid() const { return !isNull(); }
    bool isNull() const;
    bool isFuzzy() const;
    QString toString(bool withTime = true) const;
    bool hasValidTime() const;

    enum MatchType { DontMatch,
                     ExactMatch,
                     RangeMatch };
    MatchType isIncludedIn(const ImageDate &searchRange) const;
    bool includes(const Utilities::FastDateTime &date) const;

    void extendTo(const ImageDate &other);

protected:
    static QStringList monthNames();
    static QString formatRegexp();

private:
    Utilities::FastDateTime m_start, m_end;
};
}

#endif /* IMAGEDATE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
