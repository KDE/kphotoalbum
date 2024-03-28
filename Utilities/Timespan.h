// SPDX-FileCopyrightText: 2024 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef TIMESPAN_H
#define TIMESPAN_H

// Local includes
#include <DB/ImageDate.h>

// Qt includes
#include <QDebug>

// Qt classes
class QDate;
class QString;

namespace Timespan
{

struct DateDifference {
    // These represent the perceived timespan
    const int years = 0;
    const int months = 0;
    const int days = 0;

    // This is the exact number of days, used to display short periods < 2 months
    const int allDays = 0;

    bool operator==(const DateDifference &other) const
    {
        return this->allDays == other.allDays
            && this->years == other.years
            && this->months == other.months
            && this->days == other.days;
    }

    bool operator!=(const DateDifference &other) const
    {
        return this->allDays != other.allDays
            || this->years != other.years
            || this->months != other.months
            || this->days != other.days;
    }
};

DateDifference dateDifference(const QDate &date, const QDate &reference);

/**
 * @brief age computes the age of a person or entity, given a birth date and the reference date of an image.
 * @param birthDate the (exact) birth date
 * @param imageDate the (possibly fuzzy) reference date, usually of an image
 * @return a translated, formatted string describing the age or age range
 */
QString age(const QDate &birthDate, const DB::ImageDate &imageDate);
/**
 * @brief formatAge creates a textual description of a given DateDifference, interpreted as an age.
 * Colloquially speaking, this method answers the question "How old?".
 * @param age the DateDifference
 * @return a translated, formatted string
 */
QString formatAge(const DateDifference &age);

/**
 * @brief ago computes, how long ago an image was taken, based on the current date.
 * @param imageDate the (possibly fuzzy) reference date, usually of an image.
 * @return a translated, formatted string describing the timespan
 */
QString ago(const DB::ImageDate &imageDate);
/**
 * @brief formatAgo creates a textual description of a given DateDifference, interpreted as a time span.
 * Colloquially speaking, this method answers the question "How long ago?".
 * @param ago
 * @return
 */
QString formatAgo(const DateDifference &ago);
}

QDebug operator<<(QDebug debug, const Timespan::DateDifference &difference);

Q_DECLARE_METATYPE(Timespan::DateDifference)

#endif // TIMESPAN_H
