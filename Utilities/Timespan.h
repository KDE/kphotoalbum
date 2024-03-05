// SPDX-FileCopyrightText: 2024 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef TIMESPAN_H
#define TIMESPAN_H

// Local includes
#include "DB/CategoryPtr.h"
#include "DB/ImageInfoPtr.h"

// Qt includes
#include <QDebug>

// Qt classes
class QDate;
class QString;

namespace Timespan
{

struct DateDifference {
    // These represent the perceived timespan
    int years;
    int months;
    int days;

    // This is the exact number of days, used to display short periods < 2 months
    int allDays;

    bool operator==(const DateDifference &other) const
    {
        return this->years == other.years
            && this->months == other.months
            && this->days == other.days;
    }

    bool operator!=(const DateDifference &other) const
    {
        return this->years != other.years
            || this->months != other.months
            || this->days != other.days;
    }
};

DateDifference dateDifference(const QDate &date, const QDate &reference);

QString age(DB::CategoryPtr category, const QString &item, DB::ImageInfoPtr info);
QString formatAge(const DateDifference &age);

QString ago(const DB::ImageInfoPtr info);
QString formatAgo(const DateDifference &ago);
}

QDebug operator<<(QDebug debug, const Timespan::DateDifference &difference);

Q_DECLARE_METATYPE(Timespan::DateDifference)

#endif // TIMESPAN_H
