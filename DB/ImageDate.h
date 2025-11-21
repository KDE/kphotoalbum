// SPDX-FileCopyrightText: 2003 - 2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2025 Randall Rude <rsquared42@proton.me>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IMAGEDATE_H
#define IMAGEDATE_H

#include <Utilities/FastDateTime.h>
#include <QDebug>
#include <qstring.h>
#include <qstringlist.h>
#include <QTime>

namespace DB
{

class ImageDate
{
public:
    /**
     * @brief Constructs a null ImageDate.
     */
    ImageDate();
    /**
     * @brief Constructs a fuzzy ImageDate with start and end date.
     * The ImageDate cover the time range from the beginning of start date to the end of the end date.
     *
     * Note: the ImageDate will always range from the earlier to the later date, even if parameters are swapped.
     *
     * @param start the start date
     * @param end the end date
     */
    ImageDate(const QDate &start, const QDate &end);
    /**
     * @brief Constructs a (fuzzy) ImageDate with the given start and end date/time.
     * The ImageDate covers the exact time range from the start date to the end date. If start date and end date are the same, the resulting ImageDate will not be fuzzy.
     *
     * Note: the ImageDate will always range from the earlier to the later date, even if parameters are swapped.
     *
     * @param start the start date/time
     * @param end the end date/time
     */
    ImageDate(const Utilities::FastDateTime &start, const Utilities::FastDateTime &end);
    ImageDate(int yearFrom, int monthFrom, int dayFrom, int yearTo, int monthTo, int dayTo, int hourFrom, int minuteFrom, int secondFrom);
    /**
     * @brief Constructs an ImageDate for the given QDate.
     * The resulting date will be a non-fuzzy date, i.e. the start and end time will both be the same (0:00:00).
     */
    explicit ImageDate(const QDate &);
    /**
     * @brief Constructs an ImageDate for the given date and time.
     * @param date the start and end date/time
     */
    explicit ImageDate(const Utilities::FastDateTime &date);
    /**
     * @brief Constructs an ImageDate.
     *
     * If start and end are the same, or if end is not a valid QDate, an ImageDate for start and time is constructed.
     * If start and end are not the same and are valid, then time is ignored and an ImageDate for start and end is constructed.
     *
     * @param start the start date
     * @param end the end date
     * @param time the start time, if end == start of if end is invalid
     */
    ImageDate(const QDate &start, const QDate &end, const QTime &time);

    const Utilities::FastDateTime &start() const { return m_start; }
    const Utilities::FastDateTime &end() const { return m_end; }

    /**
     * @brief operator <
     * @param other
     * @return \c true, if the ImageDate starts earlier than the other ImageDate, or the ImageDate starts at the same moment as the other one, but ends earlier. \c false otherwise
     */
    bool operator<(const ImageDate &other) const;
    /**
     * @brief operator <=
     * @param other
     * @return \c true, if the ImageDate starts earlier than the other ImageDate, or the ImageDate starts at the same moment as the other one, but does not end later. \c false otherwise
     */
    bool operator<=(const ImageDate &other) const;
    /**
     * @brief operator ==
     * @param other
     * @return \c true, if both ImageDates start and end at the same time, \c false otherwise
     */
    bool operator==(const ImageDate &other) const;
    /**
     * @brief operator !=
     * @param other
     * @return \c true, if the two ImageDates are not equal.
     */
    bool operator!=(const ImageDate &other) const;

    /**
     * @brief isValid
     * @return returns \c true, if the ImageDate has at least a valid start date, \c false if it is null.
     */
    bool isValid() const { return !isNull(); }
    /**
     * @brief isNull
     * @return returns \c true, if start() is null, \c true otherwise
     */
    bool isNull() const;
    /**
     * @brief isFuzzy
     * Calling isFuzzy() on an invalid ImageDate is undefined.
     * @return \c true, if start and end is not the same, \c false otherwise.
     */
    bool isFuzzy() const;
    /**
     * @brief toString returns a localized representation of the ImageDate.
     * @param withTime can be used to control whether the time is included.
     * @return a localized QString representing the ImageDate
     */
    QString toString(bool withTime = true) const;
    /**
     * @brief hasValidTime
     * @return \c true, if the time is meaningful, \c false if the ImageDate is fuzzy.
     */
    bool hasValidTime() const;

    /**
     * @brief The MatchType enum is used to qualify the result of ImageDate::isIncludedIn(const ImageDate &)
     */
    enum class MatchType {
        NoMatch, ///< The two ImageDates do not match.
        IsContained, ///< The ImageDate is fully contained within the range.
        Overlap ///< The two ImageDates do overlap.
    };

    /**
     * @brief isIncludedIn checks whether the ImageDate is contained within the given range.
     * @param searchRange a (fuzzy) ImageDate representing the date range
     * @return MatchType::IsContained, if the ImageDate is part of the searchRange; MatchType::Overlap, if the ImageDate is not part of the searchRange, but both ranges overlap; or otherwise MatchTape::NoMatch.
     */
    MatchType isIncludedIn(const ImageDate &searchRange) const;
    /**
     * @brief The ImageDate includes another, if its start() is not later than the other start(), and its end() is not earlier than the other end().
     * If the ImageDate is not fuzzy, this is the same as comparing both dates for equality ('==').
     * @param date an ImageDate
     * @return \c true, if the date is strictly withing this ImageDate
     */
    bool includes(const Utilities::FastDateTime &date) const;

    /**
     * @brief Extends the ImageDate to incorporate the other date, usually resulting in a fuzzy date.
     *
     * If the ImageDate is invalid, this is equivalent to setting it to the other date.
     * If the other ImageDate is invalid, nothing is changed.
     * @param other the other ImageDate
     */
    void extendTo(const ImageDate &other);

private:
    Utilities::FastDateTime m_start, m_end;

    /**
     * @returns the given time with any fraction of a second zeroed
     */
    static QTime zeroMSec(const QTime& time);
};

/**
 * @brief DB::parseDateString parses a user-supplied string into a QDate.
 *
 * The dateString can be incomplete and the function will do its best to fill in the missing information.
 *
 * If \c assumeStartDate is \c true, the missing information is assumed to be the date to be at the
 * start of the imprecise time period (e.g. "2021" will be parsed as January 1st 2021). If \c assumeStartDate
 * is \c false, the missing information is assumed to be at the end of the imprecise time perion (e.g.
 * "2021" will be parsed as December 31st 2021).
 *
 * Years can be two-digit numbers. Two-digit years lower than 50 are interpreted as 20xx; other two digit years are interpreted as 19xx.
 * The general allowed format is:
 *  - day of month: a one-digit or two-digit number, possibly followed by any of '-', ',', '.', ' ', '/'
 *  - month: either a number between 1 and 12, or the full or appreviated name of the month (in English or in the current locale), possibly followed by any of '-', ',', '.', ' ', '/'
 *  - year: a two-digit or four-digit number
 *
 *  If the day is missing, it is assumed to be the first or last day of the month, depending on the parameter \c assumeStartDate.
 *  If the month is missing, it is assumed to be the first or last month of the year, depending on the parameter \c assumeStartDate.
 *  If the year is missing, it is assumed to be the current year.
 *
 * Examples:
 * ---------
 *  - parseDateString("Dec", true): QDate(currentYear, 12, 1)
 *  - parseDateString("Dec", false): QDate(currentYear, 12, 31)
 *  - parseDateString("2021", false): QDate(2021, 12, 31)
 *  - parseDateString("3. Feb. 82", true): QDate(1982, 2, 3)
 *  - parseDateString("3. Feb. 82", false): QDate(1982, 2, 3)
 *  - parseDateString("3. Feb. 12", true): QDate(2012, 2, 3)
 *  - parseDateString("3/2/12", true): QDate(2012, 2, 3)
 *  - parseDateString("03-02-12", true): QDate(2012, 2, 3)
 *
 * @param dateString a user-supplied string representing a date
 * @param assumeStartDate \c true to assume the first possible day of the month/year, \c false to assume the last possible day.
 * @return a QDate representing the parsed dateString, or an invalid QDate if the string could not be parsed.
 */
QDate parseDateString(const QString &dateString, bool assumeStartDate);
}

QDebug operator<<(QDebug debug, const DB::ImageDate &d);

#endif /* IMAGEDATE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
