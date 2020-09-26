/* Copyright (C) 2020 the KPhotoAlbum development team

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

#ifndef UTILITIES_FASTDATETIME_H
#define UTILITIES_FASTDATETIME_H
#include <QDateTime>
#include <QDate>
#include <QTime>

namespace Utilities
{

/**
 * The FastDateTime class implements a datetime that is much faster
 * than QDateTime for comparing dates.  It caches the linear time
 * since the epoch for use in comparisons.  See
 * https://bugreports.qt.io/browse/QTBUG-41714 and
 * https://bugreports.qt.io/browse/QTBUG-75585 for an explanation
 * of why the standard QDateTime is extremely slow (on the order of
 * 1 usec per comparison).
 *
 * The distinguished value INT64_MIN is used to indicate an invalid
 * date/time.  Invalid date/times are passed to QDateTime for handling.
 */

class FastDateTime
{
public:
    FastDateTime();
    FastDateTime(const QDate &d, const QTime &t, Qt::TimeSpec spec = Qt::LocalTime);
    FastDateTime(const FastDateTime &other) : m_dateTime(other.m_dateTime), m_msecsSinceEpoch(other.m_msecsSinceEpoch) {}
    // Needed for QDate(Y, M, D).startOfDay()
    FastDateTime(const QDateTime &other);
    ~FastDateTime() {};

    Q_DECL_CONSTEXPR bool operator==(const FastDateTime &other) const { if (isOK() && other.isOK()) return m_msecsSinceEpoch == other.m_msecsSinceEpoch; else return m_dateTime == other.m_dateTime; }
    Q_DECL_CONSTEXPR bool operator!=(const FastDateTime &other) const { if (isOK() && other.isOK()) return m_msecsSinceEpoch != other.m_msecsSinceEpoch; else return m_dateTime != other.m_dateTime; }
    Q_DECL_CONSTEXPR bool operator< (const FastDateTime &other) const { if (isOK() && other.isOK()) return m_msecsSinceEpoch <  other.m_msecsSinceEpoch; else return m_dateTime <  other.m_dateTime; }
    Q_DECL_CONSTEXPR bool operator<=(const FastDateTime &other) const { if (isOK() && other.isOK()) return m_msecsSinceEpoch <= other.m_msecsSinceEpoch; else return m_dateTime <= other.m_dateTime; }
    Q_DECL_CONSTEXPR bool operator> (const FastDateTime &other) const { if (isOK() && other.isOK()) return m_msecsSinceEpoch >  other.m_msecsSinceEpoch; else return m_dateTime >  other.m_dateTime; }
    Q_DECL_CONSTEXPR bool operator>=(const FastDateTime &other) const { if (isOK() && other.isOK()) return m_msecsSinceEpoch >= other.m_msecsSinceEpoch; else return m_dateTime >= other.m_dateTime; }
    bool isNull() const { return m_dateTime.isNull(); }
    bool isValid() const { return m_dateTime.isValid(); }
    QDate date() const { return m_dateTime.date(); }
    QTime time() const { return m_dateTime.time(); }
    QString toString(Qt::DateFormat format = Qt::TextDate) const { return m_dateTime.toString(format); }
    QString toString(QStringView format) const { return m_dateTime.toString(format); }
    qint64 secsTo(const FastDateTime &other) const { if (isOK() && other.isOK()) return (other.m_msecsSinceEpoch - m_msecsSinceEpoch) / 1000; else return secsTo(other); }
    qint64 toSecsSinceEpoch() const { if (isOK()) return m_msecsSinceEpoch / 1000; else return m_dateTime.toSecsSinceEpoch(); }
    FastDateTime& operator=(const Utilities::FastDateTime &other) noexcept { if (&other != this) { m_dateTime = other.m_dateTime; m_msecsSinceEpoch = other.m_msecsSinceEpoch; } return *this; }

    Q_REQUIRED_RESULT FastDateTime addDays(qint64 days) const;
    Q_REQUIRED_RESULT FastDateTime addMonths(qint64 months) const;
    Q_REQUIRED_RESULT FastDateTime addYears(qint64 years) const;
    Q_REQUIRED_RESULT FastDateTime addSecs(qint64 secs) const;

    static FastDateTime currentDateTime();
    static FastDateTime fromString(const QString &s, Qt::DateFormat f = Qt::TextDate);

private:
    Q_DECL_CONSTEXPR bool isOK() const { return m_msecsSinceEpoch > INT64_MIN; }
    QDateTime m_dateTime;
    qint64 m_msecsSinceEpoch;
};
}

#endif /* UTILITIES_FASTDATETIME_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
