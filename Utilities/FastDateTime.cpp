/* Copyright (C) 2020 the KPhotoAlbum Development Team

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

#include "FastDateTime.h"

namespace {
qint64 toValidatedMSecs(const QDateTime& dateTime) {
    return dateTime.isValid() ? dateTime.toMSecsSinceEpoch() : INT64_MIN;
}
}
Utilities::FastDateTime
Utilities::FastDateTime::addDays(qint64 days) const
{
    FastDateTime answer(*this);
    answer.m_dateTime = answer.m_dateTime.addDays(days);
    answer.m_msecsSinceEpoch = toValidatedMSecs(answer);
    return answer;
}

Utilities::FastDateTime
Utilities::FastDateTime::addMonths(qint64 months) const
{
    FastDateTime answer(*this);
    answer.m_dateTime = answer.m_dateTime.addMonths(months);
    answer.m_msecsSinceEpoch = toValidatedMSecs(answer);
    return answer;
}

Utilities::FastDateTime
Utilities::FastDateTime::addYears(qint64 years) const
{
    FastDateTime answer(*this);
    answer.m_dateTime = answer.m_dateTime.addYears(years);
    answer.m_msecsSinceEpoch = answer.m_dateTime.isValid() ?
        answer.m_dateTime.toMSecsSinceEpoch() : INT64_MIN;
    return answer;
}

Utilities::FastDateTime
Utilities::FastDateTime::addSecs(qint64 secs) const
{
    FastDateTime answer(*this);
    answer.m_dateTime = answer.m_dateTime.addSecs(secs);
    answer.m_msecsSinceEpoch = answer.m_dateTime.isValid() ?
        answer.m_dateTime.toMSecsSinceEpoch() : INT64_MIN;
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
