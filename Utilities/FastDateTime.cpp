// SPDX-FileCopyrightText: 2020 the KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2022 - 2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "FastDateTime.h"

namespace
{
static inline qint64 toValidatedMSecs(const QDateTime &dateTime)
{
    return dateTime.isValid() ? dateTime.toMSecsSinceEpoch() : INT64_MIN;
}
}

Utilities::FastDateTime::FastDateTime()
    : m_dateTime()
    , m_msecsSinceEpoch(toValidatedMSecs(m_dateTime))
{
}

Utilities::FastDateTime::FastDateTime(const QDate &d, const QTime &t,
                                      Qt::TimeSpec spec)
    : m_dateTime(d, t, spec)
    , m_msecsSinceEpoch(toValidatedMSecs(m_dateTime))
{
}

Utilities::FastDateTime::FastDateTime(const QDate &d)
    : m_dateTime(d.startOfDay())
    , m_msecsSinceEpoch(toValidatedMSecs(m_dateTime))
{
}

Utilities::FastDateTime::FastDateTime(const QDateTime &other)
    : m_dateTime(other)
    , m_msecsSinceEpoch(toValidatedMSecs(m_dateTime))
{
}

Utilities::FastDateTime
Utilities::FastDateTime::addDays(qint64 days) const
{
    FastDateTime answer(*this);
    answer.m_dateTime = answer.m_dateTime.addDays(days);
    answer.m_msecsSinceEpoch = toValidatedMSecs(answer.m_dateTime);
    return answer;
}

Utilities::FastDateTime
Utilities::FastDateTime::addMonths(qint64 months) const
{
    FastDateTime answer(*this);
    answer.m_dateTime = answer.m_dateTime.addMonths(months);
    answer.m_msecsSinceEpoch = toValidatedMSecs(answer.m_dateTime);
    return answer;
}

Utilities::FastDateTime
Utilities::FastDateTime::addYears(qint64 years) const
{
    FastDateTime answer(*this);
    answer.m_dateTime = answer.m_dateTime.addYears(years);
    answer.m_msecsSinceEpoch = toValidatedMSecs(answer.m_dateTime);
    return answer;
}

Utilities::FastDateTime
Utilities::FastDateTime::addSecs(qint64 secs) const
{
    FastDateTime answer(*this);
    answer.m_dateTime = answer.m_dateTime.addSecs(secs);
    answer.m_msecsSinceEpoch = toValidatedMSecs(answer.m_dateTime);
    return answer;
}

Utilities::FastDateTime
Utilities::FastDateTime::currentDateTime()
{
    QDateTime answer(QDateTime::currentDateTime());
    return FastDateTime(answer);
}

Utilities::FastDateTime
Utilities::FastDateTime::fromString(const QString &s, Qt::DateFormat f)
{
    QDateTime answer(QDateTime::fromString(s, f));
    return FastDateTime(answer);
}

QDebug operator<<(QDebug debug, const Utilities::FastDateTime &d)
{
    QDebugStateSaver saveState(debug);
    if (d.isNull()) {
        debug.nospace() << "Utilities::FastDateTime()";
    } else {
        debug.nospace() << "Utilities::FastDateTime(" << d.toString(Qt::ISODate) << ")";
    }
    return debug;
}
