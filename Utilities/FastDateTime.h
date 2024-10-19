// SPDX-FileCopyrightText: 2020 the KPhotoAlbum development team
// SPDX-FileCopyrightText: 2022-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2024 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef UTILITIES_FASTDATETIME_H
#define UTILITIES_FASTDATETIME_H
#include <QDate>
#include <QDateTime>
#include <QDebug>
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
    // FIXME: KF6 port: The "Qt::TimeSpec spec = Qt::LocalTime" parameter used in the removed
    // QDateTime::QDateTime(QDate date, QTime time, Qt::TimeSpec spec, int offsetSeconds = 0)
    // overload doesn't seem to be used anywhere.
    // Please somebody review if we really can simply use QDateTime(date, time) here instead!
    FastDateTime(const QDate &d, const QTime &t);
    FastDateTime(const QDate &d);
    FastDateTime(const FastDateTime &other) = default;
    FastDateTime(FastDateTime &&other) = default;
    // Needed for QDate(Y, M, D).startOfDay()
    FastDateTime(const QDateTime &other);
    ~FastDateTime() {};

    Q_DECL_CONSTEXPR bool operator==(const FastDateTime &other) const
    {
        if (isOK() && other.isOK())
            return m_msecsSinceEpoch == other.m_msecsSinceEpoch;
        else
            return m_dateTime == other.m_dateTime;
    }
    Q_DECL_CONSTEXPR bool operator!=(const FastDateTime &other) const
    {
        if (isOK() && other.isOK())
            return m_msecsSinceEpoch != other.m_msecsSinceEpoch;
        else
            return m_dateTime != other.m_dateTime;
    }
    Q_DECL_CONSTEXPR bool operator<(const FastDateTime &other) const
    {
        if (isOK() && other.isOK())
            return m_msecsSinceEpoch < other.m_msecsSinceEpoch;
        else
            return m_dateTime < other.m_dateTime;
    }
    Q_DECL_CONSTEXPR bool operator<=(const FastDateTime &other) const
    {
        if (isOK() && other.isOK())
            return m_msecsSinceEpoch <= other.m_msecsSinceEpoch;
        else
            return m_dateTime <= other.m_dateTime;
    }
    Q_DECL_CONSTEXPR bool operator>(const FastDateTime &other) const
    {
        if (isOK() && other.isOK())
            return m_msecsSinceEpoch > other.m_msecsSinceEpoch;
        else
            return m_dateTime > other.m_dateTime;
    }
    Q_DECL_CONSTEXPR bool operator>=(const FastDateTime &other) const
    {
        if (isOK() && other.isOK())
            return m_msecsSinceEpoch >= other.m_msecsSinceEpoch;
        else
            return m_dateTime >= other.m_dateTime;
    }
    bool isNull() const { return m_dateTime.isNull(); }
    bool isValid() const { return m_dateTime.isValid(); }
    QDate date() const { return m_dateTime.date(); }
    QTime time() const { return m_dateTime.time(); }
    QString toString(Qt::DateFormat format = Qt::TextDate) const { return m_dateTime.toString(format); }
    QString toString(QStringView format) const
    {
        return m_dateTime.toString(format);
    }
    qint64 secsTo(const FastDateTime &other) const
    {
        if (isOK() && other.isOK())
            return (other.m_msecsSinceEpoch - m_msecsSinceEpoch) / 1000;
        else
            return secsTo(other);
    }
    qint64 toSecsSinceEpoch() const
    {
        if (isOK())
            return m_msecsSinceEpoch / 1000;
        else
            return m_dateTime.toSecsSinceEpoch();
    }
    FastDateTime &operator=(const Utilities::FastDateTime &other) = default;
    FastDateTime &operator=(Utilities::FastDateTime &&other) = default;

    Q_REQUIRED_RESULT FastDateTime addDays(qint64 days) const;
    Q_REQUIRED_RESULT FastDateTime addMonths(qint64 months) const;
    Q_REQUIRED_RESULT FastDateTime addYears(qint64 years) const;
    Q_REQUIRED_RESULT FastDateTime addSecs(qint64 secs) const;

    static FastDateTime currentDateTime();
    static FastDateTime fromString(const QString &s, Qt::DateFormat f = Qt::TextDate);

private:
    Q_DECL_CONSTEXPR bool isOK() const
    {
        return m_msecsSinceEpoch > INT64_MIN;
    }
    QDateTime m_dateTime;
    qint64 m_msecsSinceEpoch;
};
}

QDebug operator<<(QDebug debug, const Utilities::FastDateTime &d);

#endif /* UTILITIES_FASTDATETIME_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
