/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>
   SPDX-FileCopyrightText: 2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMAGEDATE_H
#define IMAGEDATE_H
#include <Utilities/FastDateTime.h>
#include <QDebug>
#include <qstring.h>
#include <qstringlist.h>

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
    static QDate parseDate(const QString &date, bool startDate);

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

    enum MatchType { DontMatch,
                     ExactMatch,
                     RangeMatch };

    /**
     * @brief isIncludedIn
     * @param searchRange
     * @return
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

protected:
    static QStringList monthNames();
    static QString formatRegexp();

private:
    Utilities::FastDateTime m_start, m_end;
};
}

QDebug operator<<(QDebug debug, const DB::ImageDate &d);

#endif /* IMAGEDATE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
