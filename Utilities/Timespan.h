// SPDX-FileCopyrightText: 2024-2026 Tobias Leupold <tl@stonemx.de>
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

/**
 * @brief dateDifference computes the perceived difference between two dates, in years, months and
 * days. This does, in some cases, deviate from the QDate arithmetics.
 * @param date a date that should be compared to reference, e.g. some date in the past
 * @param reference the reference date to compare to, e.g. today
 * @return a DateDifference object, containg the number of years, months and days passed between the
 * input dates, as well as the (arithmetically correct) number of days between them
 */
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
 * @brief ago computes, how long ago an image was taken, based on the reference date, and formats the difference using the formatAgo function.
 *
 * @param imageDate the (possibly fuzzy) reference date, usually of an image.
 * @param reference the reference date, usually the current date
 * @return a translated, formatted string describing the timespan
 */
QString ago(const DB::ImageDate &imageDate, const QDate &reference = QDate::currentDate());

/**
 * @brief formatAgo creates a textual description of a given DateDifference, interpreted as a time span.
 * Colloquially speaking, this method answers the question "How long ago?".
 *
 * The result is less accurate the farther the time difference is, rounded to the nearest unit:
 * - less than 2 days ago: today / yesterday
 * - 3 to 13 days: "X days"
 * - 14 days to <2 months: "X weeks"
 * - 2 months to <1 year: "X months"
 * - 1 year to < 10 years: "X years" or "X years Y months"
 * - >= 10 years: "X years"
 *
 * @param ago
 * @return a translated string describing the timestamp (e.g. "yesterday", "5 days", or "1 year 2 months")
 */
QString formatAgo(const DateDifference &ago);
}

QDebug operator<<(QDebug debug, const Timespan::DateDifference &difference);

Q_DECLARE_METATYPE(Timespan::DateDifference)

#endif // TIMESPAN_H
