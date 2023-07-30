// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2022-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ImageDate.h"

#include <KLocalizedString>
#include <QDebug>
#include <QLocale>
#include <qregexp.h>

using namespace DB;

static const QTime _startOfDay_(0, 0, 0);
static const QTime _endOfDay_(23, 59, 59);

namespace
{

QStringList monthNames()
{
    static QStringList res;
    if (res.isEmpty()) {
        for (int i = 1; i <= 12; ++i) {
            res << QLocale().standaloneMonthName(i, QLocale::ShortFormat);
        }
        for (int i = 1; i <= 12; ++i) {
            res << QLocale().standaloneMonthName(i, QLocale::LongFormat);
        }

        res << i18nc("Abbreviated month name", "jan") << i18nc("Abbreviated month name", "feb")
            << i18nc("Abbreviated month name", "mar") << i18nc("Abbreviated month name", "apr")
            << i18nc("Abbreviated month name", "may") << i18nc("Abbreviated month name", "jun")
            << i18nc("Abbreviated month name", "jul") << i18nc("Abbreviated month name", "aug")
            << i18nc("Abbreviated month name", "sep") << i18nc("Abbreviated month name", "oct")
            << i18nc("Abbreviated month name", "nov") << i18nc("Abbreviated month name", "dec");
        res << QString::fromLatin1("jan") << QString::fromLatin1("feb") << QString::fromLatin1("mar") << QString::fromLatin1("apr")
            << QString::fromLatin1("may") << QString::fromLatin1("jun") << QString::fromLatin1("jul") << QString::fromLatin1("aug")
            << QString::fromLatin1("sep") << QString::fromLatin1("oct") << QString::fromLatin1("nov") << QString::fromLatin1("dec");

        for (int i = 1; i <= 12; ++i) {
            res << QLocale().monthName(i, QLocale::ShortFormat);
        }
        for (int i = 1; i <= 12; ++i) {
            res << QLocale().monthName(i, QLocale::LongFormat);
        }

        for (QStringList::iterator it = res.begin(); it != res.end(); ++it)
            *it = it->toLower();
    }
    return res;
}

QString formatRegexp()
{
    static QString str;
    if (str.isEmpty()) {
        str = QString::fromLatin1("^((\\d\\d?)([-. /]+|$))?((");
        QStringList months = monthNames();
        for (QStringList::ConstIterator monthIt = months.constBegin(); monthIt != months.constEnd(); ++monthIt)
            str += QString::fromLatin1("%1|").arg(*monthIt);

        str += QString::fromLatin1("\\d?\\d)([-. /]+|$))?(\\d\\d(\\d\\d)?)?$");
    }
    return str;
}

} // namespace

ImageDate::ImageDate(const QDate &date)
    : m_start(date, _startOfDay_)
    , m_end(m_start)
{
}

ImageDate::ImageDate(const Utilities::FastDateTime &date)
    : m_start(date)
    , m_end(date)
{
}

bool ImageDate::operator<=(const ImageDate &other) const
{
    // This operator is used by QMap when checking for equal elements, thus we need the second part too.
    return m_start < other.m_start || (m_start == other.m_start && m_end <= other.m_end);
}

ImageDate::ImageDate()
    : m_start()
    , m_end()
{
}

bool ImageDate::isNull() const
{
    return m_start.isNull();
}

bool ImageDate::isFuzzy() const
{
    return m_start != m_end;
}

static bool isFirstSecOfMonth(const Utilities::FastDateTime &date)
{
    return date.date().day() == 1 && date.time().hour() == 0 && date.time().minute() == 0;
}

static bool isLastSecOfMonth(Utilities::FastDateTime date)
{
    return isFirstSecOfMonth(date.addSecs(1));
}

static bool isFirstSecOfDay(const Utilities::FastDateTime &time)
{
    return time.time().hour() == 0 && time.time().minute() == 0 && time.time().second() == 0;
}

static bool isLastSecOfDay(const Utilities::FastDateTime &time)
{
    return time.time().hour() == 23 && time.time().minute() == 59 && time.time().second() == 59;
}

QString ImageDate::toString(bool withTime) const
{
    if (m_start.isNull())
        return QString();

    if (m_start == m_end) {
        if (withTime && !isFirstSecOfDay(m_start))
            return m_start.toString(QString::fromLatin1("d. MMM yyyy hh:mm:ss"));
        else
            return m_start.toString(QString::fromLatin1("d. MMM yyyy"));
    }

    // start is different from end.
    if (isFirstSecOfMonth(m_start) && isLastSecOfMonth(m_end)) {
        if (m_start.date().month() == 1 && m_end.date().month() == 12) {
            if (m_start.date().year() == m_end.date().year()) {
                // 2005
                return QString::number(m_start.date().year());
            } else {
                // 2005-2006
                return QString::fromLatin1("%1 - %2").arg(m_start.date().year()).arg(m_end.date().year());
            }
        } else {
            // a whole month, but not a whole year.
            if (m_start.date().year() == m_end.date().year() && m_start.date().month() == m_end.date().month()) {
                // jan 2005
                return QString::fromLatin1("%1 %2")
                    .arg(QLocale().standaloneMonthName(m_start.date().month(), QLocale::ShortFormat))
                    .arg(m_start.date().year());
            } else {
                // jan 2005 - feb 2006
                return QString::fromLatin1("%1 %2 - %3 %4")
                    .arg(QLocale().standaloneMonthName(m_start.date().month(), QLocale::ShortFormat))
                    .arg(m_start.date().year())
                    .arg(QLocale().standaloneMonthName(m_end.date().month(), QLocale::ShortFormat))
                    .arg(m_end.date().year());
            }
        }
    }

    if (!withTime || (isFirstSecOfDay(m_start) && isLastSecOfDay(m_end))) {
        if (m_start.date() == m_end.date()) {
            // A whole day
            return m_start.toString(QString::fromLatin1("d. MMM yyyy"));
        } else {
            // A day range
            return QString::fromLatin1("%1 - %2")
                .arg(m_start.toString(QString::fromLatin1("d. MMM yyyy")), m_end.toString(QString::fromLatin1("d. MMM yyyy")));
        }
    }

    // Range smaller than one day.
    if (withTime && (!isFirstSecOfDay(m_start) || !isLastSecOfDay(m_end)))
        return QString::fromLatin1("%1 - %2")
            .arg(m_start.toString(QString::fromLatin1("d. MMM yyyy hh:mm")), m_end.toString(QString::fromLatin1("d. MMM yyyy hh:mm")));
    else
        return QString::fromLatin1("%1 - %2")
            .arg(m_start.toString(QString::fromLatin1("d. MMM yyyy")), m_end.toString(QString::fromLatin1("d. MMM yyyy")));
}

bool ImageDate::operator==(const ImageDate &other) const
{
    return m_start == other.m_start && m_end == other.m_end;
}

bool ImageDate::operator!=(const ImageDate &other) const
{
    return !(*this == other);
}

bool ImageDate::operator<(const ImageDate &other) const
{
    return start() < other.start() || (start() == other.start() && end() < other.end());
}

ImageDate::ImageDate(const Utilities::FastDateTime &start, const Utilities::FastDateTime &end)
{
    if (!start.isValid() || !end.isValid() || start <= end) {
        m_start = start;
        m_end = end;
    } else {
        m_start = end;
        m_end = start;
    }
}

ImageDate::ImageDate(const QDate &start, const QDate &end)
{
    if (!start.isValid() || !end.isValid() || start <= end) {
        m_start = Utilities::FastDateTime(start, _startOfDay_);
        m_end = Utilities::FastDateTime(end, _endOfDay_);
    } else {
        m_start = Utilities::FastDateTime(end, _startOfDay_);
        m_end = Utilities::FastDateTime(start, _endOfDay_);
    }
}

static QDate addMonth(int year, int month)
{
    if (month == 12) {
        year++;
        month = 1;
    } else
        month++;
    return QDate(year, month, 1);
}

ImageDate::ImageDate(int yearFrom, int monthFrom, int dayFrom, int yearTo, int monthTo, int dayTo, int hourFrom, int minuteFrom, int secondFrom)
{
    if (yearFrom <= 0) {
        m_start = Utilities::FastDateTime();
        m_end = Utilities::FastDateTime();
        return;
    }

    if (monthFrom <= 0) {
        m_start = QDate(yearFrom, 1, 1).startOfDay();
        m_end = QDate(yearFrom + 1, 1, 1).startOfDay().addSecs(-1);
    } else if (dayFrom <= 0) {
        m_start = QDate(yearFrom, monthFrom, 1).startOfDay();
        m_end = addMonth(yearFrom, monthFrom).startOfDay().addSecs(-1);
    } else if (hourFrom < 0) {
        m_start = QDate(yearFrom, monthFrom, dayFrom).startOfDay();
        m_end = QDate(yearFrom, monthFrom, dayFrom).addDays(1).startOfDay().addSecs(-1);
    } else if (minuteFrom < 0) {
        m_start = Utilities::FastDateTime(QDate(yearFrom, monthFrom, dayFrom), QTime(hourFrom, 0, 0));
        m_end = Utilities::FastDateTime(QDate(yearFrom, monthFrom, dayFrom), QTime(hourFrom, 23, 59));
    } else if (secondFrom < 0) {
        m_start = Utilities::FastDateTime(QDate(yearFrom, monthFrom, dayFrom), QTime(hourFrom, minuteFrom, 0));
        m_end = Utilities::FastDateTime(QDate(yearFrom, monthFrom, dayFrom), QTime(hourFrom, minuteFrom, 59));
    } else {
        m_start = Utilities::FastDateTime(QDate(yearFrom, monthFrom, dayFrom), QTime(hourFrom, minuteFrom, secondFrom));
        m_end = m_start;
    }

    if (yearTo > 0) {
        m_end = QDate(yearTo + 1, 1, 1).startOfDay().addSecs(-1);

        if (monthTo > 0) {
            m_end = addMonth(yearTo, monthTo).startOfDay().addSecs(-1);

            if (dayTo > 0) {
                if (dayFrom == dayTo && monthFrom == monthTo && yearFrom == yearTo)
                    m_end = m_start;
                else
                    m_end = QDate(yearTo, monthTo, dayTo).addDays(1).startOfDay().addSecs(-1);
            }
        }
        // It should not be possible here for m_end < m_start.
        Q_ASSERT(m_start <= m_end);
    }
}

bool ImageDate::hasValidTime() const
{
    return m_start == m_end;
}

ImageDate::ImageDate(const QDate &start, const QDate &end, const QTime &time)
{
    const QDate validatedEnd = (end.isValid()) ? end : start;

    if (start == validatedEnd && time.isValid()) {
        m_start = Utilities::FastDateTime(start, time);
        m_end = m_start;
    } else {
        if (start > validatedEnd) {
            m_end = Utilities::FastDateTime(start, _startOfDay_);
            m_start = Utilities::FastDateTime(validatedEnd, _endOfDay_);
        } else {
            m_start = Utilities::FastDateTime(start, _startOfDay_);
            m_end = Utilities::FastDateTime(validatedEnd, _endOfDay_);
        }
    }
}

ImageDate::MatchType ImageDate::isIncludedIn(const ImageDate &searchRange) const
{
    if (searchRange.start() <= start() && searchRange.end() >= end())
        return MatchType::IsContained;

    if (searchRange.start() <= end() && searchRange.end() >= start()) {
        return MatchType::Overlap;
    }
    return MatchType::NoMatch;
}

bool ImageDate::includes(const Utilities::FastDateTime &date) const
{
    return ImageDate(date).isIncludedIn(*this) == MatchType::IsContained;
}

void ImageDate::extendTo(const ImageDate &other)
{
    if (other.isNull())
        return;

    if (isNull()) {
        m_start = other.m_start;
        m_end = other.m_end;
    } else {
        if (other.m_start < m_start)
            m_start = other.m_start;
        if (other.m_end > m_end)
            m_end = other.m_end;
    }
}

QDate DB::parseDateString(const QString &dateString, bool assumeStartDate)
{
    QRegExp regexp(formatRegexp(), Qt::CaseInsensitive);

    if (regexp.exactMatch(dateString)) {
        int year = 0;
        int month = 0;
        int day = 0;

        QString dayStr = regexp.cap(2);
        QString monthStr = regexp.cap(5).toLower();
        QString yearStr = regexp.cap(7);

        if (dayStr.length() != 0)
            day = dayStr.toInt();

        if (yearStr.length() != 0) {
            year = yearStr.toInt();
            if (year < 50)
                year += 2000;
            if (year < 100)
                year += 1900;
        }
        if (monthStr.length() != 0) {
            int index = monthNames().indexOf(monthStr);
            if (index != -1)
                month = (index % 12) + 1;
            else
                month = monthStr.toInt();
        }
        if (year == 0)
            year = QDate::currentDate().year();
        if (month == 0) {
            if (assumeStartDate) {
                month = 1;
                day = 1;
            } else {
                month = 12;
                day = 31;
            }
        } else if (day == 0) {
            if (assumeStartDate)
                day = 1;
            else
                day = QDate(year, month, 1).daysInMonth();
        }
        return QDate(year, month, day);
    } else
        return QDate();
}

QDebug operator<<(QDebug debug, const DB::ImageDate &d)
{
    QDebugStateSaver saveState(debug);

    if (d.isNull()) {
        debug << "DB::ImageDate()";
    } else if (d.isFuzzy()) {
        debug.nospace() << "DB::ImageDate(" << d.start().date().toString(Qt::ISODate) << ", " << d.end().date().toString(Qt::ISODate) << ")";
    } else {
        debug.nospace() << "DB::ImageDate(" << d.start().date().toString(Qt::ISODate) << ")";
    }
    return debug;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
