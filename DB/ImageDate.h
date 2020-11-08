/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
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

    const Utilities::FastDateTime &start() const { return m_start; }
    const Utilities::FastDateTime &end() const { return m_end; }
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
