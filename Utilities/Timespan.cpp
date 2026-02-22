// SPDX-FileCopyrightText: 2026 Randall Rude <rsquared42@proton.me>
// SPDX-FileCopyrightText: 2024-2026 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

// Local includes
#include "Timespan.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QDate>
#include <QDebug>

// C++ includes
#include <cmath>

Timespan::DateDifference Timespan::dateDifference(const QDate &date, const QDate &reference)
{
    if (date > reference) {
        return dateDifference(reference, date);
    }

    int dateDay = date.day();
    if (date.month() == 2 && dateDay == 29
        && !QDate::isLeapYear(reference.year())) {
        // If we calculate the timespan to a February 29 for a non-leap year, we use February 28
        // instead (the last day in February). This will also make birthdays for people born on
        // February 29 being calculated correctly (February 28, the last day in February, for
        // non-leap years)
        dateDay = 28;
    }

    int years = reference.year() - date.year();
    int months = reference.month() - date.month();
    if (reference.month() < date.month()
        || ((reference.month() == date.month()) && (reference.day() < dateDay))) {
        years--;
        months += 12;
    }
    if (reference.day() < dateDay) {
        months--;
    }

    int remainderMonth = reference.month() - (reference.day() < dateDay);
    int remainderYear = reference.year();
    if (remainderMonth == 0) {
        remainderMonth = 12;
        remainderYear--;
    }

    const auto daysOfRemainderMonth = QDate(remainderYear, remainderMonth, 1).daysInMonth();
    const auto remainderDay = dateDay > daysOfRemainderMonth ? daysOfRemainderMonth : dateDay;

    int days = QDate(remainderYear, remainderMonth, remainderDay).daysTo(reference);

    return {
        years,
        months,
        days,
        int(date.daysTo(reference))
    };
}

QString Timespan::age(const QDate &birthDate, const DB::ImageDate &imageDate)
{
    if (!birthDate.isValid() || !imageDate.isValid()) {
        return QString();
    }

    const auto dateStart = imageDate.start().date();
    const auto dateEnd = imageDate.end().date();

    if (dateStart < birthDate && dateEnd < birthDate) {
        // It's a photo of a person before their birth
        return QString();

    } else if (dateStart < birthDate && dateEnd >= birthDate) {
        // At least the end date is after the person's birth
        const auto maxAge = dateDifference(birthDate, dateEnd);
        return i18nc("Like \"up to 6 years\" old",
                     " (up to %1)",
                     formatAge(maxAge));
    }

    // The photo was taken after the person's birth
    const auto minAge = formatAge(dateDifference(birthDate, dateStart));
    const auto maxAge = formatAge(dateDifference(birthDate, dateEnd));

    if (minAge == maxAge) {
        return i18nc("Like \"6 years\" old", " (%1)", minAge);
    } else {
        return i18nc("Like \"6 years to 7 years\" old", " (%1 to %2)", minAge, maxAge);
    }
}

QString Timespan::formatAge(const Timespan::DateDifference &age)
{
    if (age.years == 0 && age.months == 0) {
        // For the really small ones, we output days :-)
        return i18ncp("Like \"The baby is \'8 days\' old\"",
                      "%1 day", "%1 days", age.days);

    } else if (age.years == 0) {
        // Below one year, we output the months
        return i18ncp("Like \"The baby is \'4 months\' old\"",
                      "%1 month", "%1 months", age.months);

    } else if (age.years < 3) {
        // For toddlers, we output years and months
        const auto yearsString = i18ncp("Like \"The person is \'28 years\' old\"",
                                        "%1 year", "%1 years", age.years);
        if (age.months == 0) {
            return yearsString;
        } else {
            const auto monthsString = i18ncp("Like \"The baby is \"4 months\" old\"",
                                             "%1 month", "%1 months", age.months);
            return i18nc("This combines an age of a person in years (%1) with the additional "
                         "months (%2), like \"The person is \'1 year and 3 months\' old\"",
                         "%1, %2", yearsString, monthsString);
        }

    } else {
        // For persons at least three years old, we return the age in (full) years
        return QString::number(age.years);
    }
}

QString Timespan::ago(const DB::ImageDate &imageDate, const QDate &reference)
{
    if (!imageDate.isValid()) {
        return QString();
    }

    const auto dateStart = imageDate.start().date();
    const auto dateEnd = imageDate.end().date();

    if (reference < dateStart && reference < dateEnd) {
        // It's a photo not taken yet ;-)
        return QString();
    }

    const auto minAgo = dateDifference(dateEnd, reference);
    const auto maxAgo = dateDifference(dateStart, reference);

    if (minAgo == maxAgo) {
        if (minAgo.allDays == 0) {
            // The photo has been taken exactly today --> leave out the "ago"
            return i18n(" (today)");
        } else if (minAgo.allDays == 1) {
            // The photo has been taken exactly yesterday --> leave out the "ago"
            return i18n(" (yesterday)");
        } else {
            // Let formatAgo do the formatting and add "ago"
            return i18nc("Like \"6 years ago\"",
                         " (%1 ago)", formatAgo(minAgo));
        }
    } else {
        return i18nc("Like \"6 to 7 years ago\"",
                     " (%1 to %2 ago)", formatAgo(minAgo), formatAgo(maxAgo));
    }
}

QString Timespan::formatAgo(const Timespan::DateDifference &ago)
{
    if (ago.allDays == 0) {
        return i18n("today");

    } else if (ago.allDays == 1) {
        return i18n("yesterday");

    } else if (ago.allDays < 14) {
        // Less than two weeks --> we display days
        return i18ncp("Like \"This happened \'6 days\' ago\"",
                      "%1 day", "%1 days", ago.allDays);

    } else if (ago.years == 0 && ago.months < 2) {
        // Less than 2 months. Depending on the number of weeks,
        // we either format weeks or we round to months.

        const auto caWeeks = int(std::round(ago.allDays / 7.0));

        if (caWeeks <= 8) {
            // We format weeks
            if (ago.allDays % 7 == 0) {
                // We have an exact amount of weeks
                return i18ncp("Like \"This happened \'3 weeks\' ago\"",
                              "%1 week", "%1 weeks", ago.allDays / 7);

            } else {
                // We calculate a "ca." amount of weeks
                return i18ncp("Like \"This happened \'ca. 6 weeks\' ago\"",
                              "ca. %1 week", "ca. %1 weeks", caWeeks);
            }
        } else {
            // We round up to "2 months"
            return i18ncp("Like \"This happened \'ca. 2 months\' ago\"",
                          "ca. %1 month", "ca. %1 months", 2);
        }

    } else if (ago.years == 0) {
        // Less than a year --> we display (ca. months)
        if (ago.days == 0) {
            // We have an exact amount of months
            return i18ncp("Like \"This happened \'2 months\' ago\"",
                          "%1 month", "%1 months", ago.months);

        } else if (ago.days <= 23) {
            // Ca. one week to the next month --> we display the counted months
            return i18ncp("Like \"This happened \'ca. 2 months\' ago\"",
                          "ca. %1 month", "ca. %1 months", ago.months);
        } else {
            // Likely less than a week to the next month --> we add one more
            if (ago.months + 1 < 12) {
                return i18ncp("Like \"This happened \'ca. 2 months\' ago\"",
                              "ca. %1 month", "ca. %1 months", ago.months + 1);
            } else {
                // In case we complete the first year with this, we display "1 year",
                // using the same translations string as for more years
                return i18ncp("Like \"This happened \'2 years\' ago\"",
                              "%1 year", "%1 years", 1);
            }
        }

    } else {
        // At least one year ago --> we display years and months
        auto years = ago.years;
        auto months = ago.months;
        if (ago.days > 23) {
            // Likely less than a week to the next month --> we add one more
            months++;
            if (months == 12) {
                months = 0;
                years++;
            }
        }

        if (months == 0) {
            return i18ncp("Like \"This happened \'2 years\' ago\"",
                          "%1 year", "%1 years", years);
        } else {
            const auto formattedYears = i18ncp("Like \"This happened \'2 years\' ago\"",
                                               "%1 year", "%1 years", years);
            const auto formattedMonths = i18ncp("Like \"This happened \'2 months\' ago\"",
                                                "%1 month", "%1 months", months);
            return i18nc("This combines a number of years and months to a timespan, like \"This "
                         "happened 3 years and 1 month\" ago, where the first parameter is the "
                         "formatted years string and the second one is the formatted months string",
                         "%1 and %2", formattedYears, formattedMonths);
        }
    }
}

QDebug operator<<(QDebug debug, const Timespan::DateDifference &difference)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "DateDifference("
                    << "years: " << difference.years
                    << ", months: " << difference.months
                    << ", days: " << difference.days
                    << " - total days: " << difference.allDays
                    << ')';
    return debug;
}
