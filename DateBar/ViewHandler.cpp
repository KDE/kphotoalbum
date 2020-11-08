/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ViewHandler.h"

#include <KLocalizedString>
#include <QLocale>
#include <math.h>

using namespace DateBar;

/**
 * \class DateBar::ViewHandler
 * \brief Base class for classes handling logic regarding individual bars in the datebar.
 *
 * A major part of the date bar is figuring out which date interval a given bar represent, this class is taking care of this.
 */

/**
 * Indicate that the first unit in the bar represent the date given as parameter
 */
void ViewHandler::init(const Utilities::FastDateTime &startDate)
{
    m_startDate = startDate;
}

/**
 * Returns whether this unit is a major unit (like January in year view, or Monday in week view)
 * This information is used to draw a longer line in the date bar for the given unit.
 */
bool DateBar::ViewHandler::isMajorUnit(int)
{
    // included for documentation
    return true;
}

/**
 * Returns whether this is a mid unit (like 30 minute in hour view)
 * This information is used to draw a slightly longer line in the date bar for the given unit.
 */
bool ViewHandler::isMidUnit(int /*unit*/)
{
    return false;
}

/**
 * Returns the text to be shown for the given unit. This method will only be call for major units.
 */
QString DateBar::ViewHandler::text(int)
{
    // Included for documentation.
    return QString();
}

/**
 * Returns the length of one unit (to be shown at the right of the date bar)
 */
QString DateBar::ViewHandler::unitText() const
{
    // Included for documentation.
    return QString();
}

/**
 * Return the date for the beginning of the unit given as the first argument. If the second optional argument is
 * given, then this is used as the date for the first unit, otherwise the date given to \ref init will be used as offset.
 */
Utilities::FastDateTime DateBar::ViewHandler::date(int, Utilities::FastDateTime)
{
    // Included for documentation.
    return Utilities::FastDateTime();
}

void DecadeViewHandler::init(const Utilities::FastDateTime &startDate)
{
    Utilities::FastDateTime date = Utilities::FastDateTime(QDate(startDate.date().year(), 1, 1), QTime(0, 0, 0));
    ViewHandler::init(date);
}

bool DecadeViewHandler::isMajorUnit(int unit)
{
    return date(unit).date().year() % 10 == 0;
}

bool DecadeViewHandler::isMidUnit(int unit)
{
    return date(unit).date().year() % 5 == 0;
}

QString DecadeViewHandler::text(int unit)
{
    return QString::number(date(unit).date().year());
}

Utilities::FastDateTime DecadeViewHandler::date(int unit, Utilities::FastDateTime reference)
{
    if (reference.isNull())
        reference = m_startDate;
    return reference.addMonths(12 * unit);
}

QString DecadeViewHandler::unitText() const
{
    return i18n("1 Year");
}

void YearViewHandler::init(const Utilities::FastDateTime &startDate)
{
    Utilities::FastDateTime date = Utilities::FastDateTime(QDate(startDate.date().year(), startDate.date().month(), 1), QTime(0, 0, 0));
    ViewHandler::init(date);
}

bool YearViewHandler::isMajorUnit(int unit)
{
    return date(unit).date().month() == 1;
}

bool YearViewHandler::isMidUnit(int unit)
{
    return date(unit).date().month() == 7;
}

QString YearViewHandler::text(int unit)
{
    return QString::number(date(unit).date().year());
}

Utilities::FastDateTime YearViewHandler::date(int unit, Utilities::FastDateTime reference)
{
    if (reference.isNull())
        reference = m_startDate;
    return reference.addMonths(unit);
}

QString YearViewHandler::unitText() const
{
    return i18n("1 Month");
}

void MonthViewHandler::init(const Utilities::FastDateTime &startDate)
{
    QDate date = startDate.date().addDays(-startDate.date().dayOfWeek() + 1); // Wind to monday
    ViewHandler::init(Utilities::FastDateTime(date, QTime(0, 0, 0)));
}

bool MonthViewHandler::isMajorUnit(int unit)
{
    return date(unit).date().day() <= 7;
}

QString MonthViewHandler::text(int unit)
{
    static int lastunit = 99999;
    static int printedLast = false;
    if (unit < lastunit)
        printedLast = true;
    QString str;
    if (!printedLast)
        str = QLocale().toString(date(unit).date(), QLocale::ShortFormat);
    printedLast = !printedLast;
    lastunit = unit;
    return str;
}

Utilities::FastDateTime MonthViewHandler::date(int unit, Utilities::FastDateTime reference)
{
    if (reference.isNull())
        reference = m_startDate;
    return reference.addDays(7 * unit);
}

QString MonthViewHandler::unitText() const
{
    return i18n("1 Week");
}

void WeekViewHandler::init(const Utilities::FastDateTime &startDate)
{
    ViewHandler::init(Utilities::FastDateTime(startDate.date(), QTime(0, 0, 0)));
}

bool WeekViewHandler::isMajorUnit(int unit)
{
    return date(unit).date().dayOfWeek() == 1;
}

QString WeekViewHandler::text(int unit)
{
    return QLocale().toString(date(unit).date(), QLocale::ShortFormat);
}

Utilities::FastDateTime WeekViewHandler::date(int unit, Utilities::FastDateTime reference)
{
    if (reference.isNull())
        reference = m_startDate;
    return reference.addDays(unit);
}

QString WeekViewHandler::unitText() const
{
    return i18n("1 Day");
}

void DayViewHandler::init(const Utilities::FastDateTime &startDate)
{
    Utilities::FastDateTime date = startDate;
    if (date.time().hour() % 2)
        date = date.addSecs(60 * 60);

    ViewHandler::init(Utilities::FastDateTime(date.date(), QTime(date.time().hour(), 0, 0)));
}

bool DayViewHandler::isMajorUnit(int unit)
{
    int h = date(unit).time().hour();
    return h == 0 || h == 12;
}

bool DayViewHandler::isMidUnit(int unit)
{
    int h = date(unit).time().hour();
    return h == 6 || h == 18;
}

QString DayViewHandler::text(int unit)
{
    if (date(unit).time().hour() == 0)
        return QLocale().toString(date(unit).date(), QLocale::ShortFormat);
    else
        return date(unit).toString(QString::fromLatin1("h:00"));
}

Utilities::FastDateTime DayViewHandler::date(int unit, Utilities::FastDateTime reference)
{
    if (reference.isNull())
        reference = m_startDate;
    return reference.addSecs(2 * 60 * 60 * unit);
}

QString DayViewHandler::unitText() const
{
    return i18n("2 Hours");
}

void HourViewHandler::init(const Utilities::FastDateTime &startDate)
{
    ViewHandler::init(Utilities::FastDateTime(startDate.date(),
                                              QTime(startDate.time().hour(), 10 * (int)floor(startDate.time().minute() / 10.0), 0)));
}

bool HourViewHandler::isMajorUnit(int unit)
{
    return date(unit).time().minute() == 0;
}

bool HourViewHandler::isMidUnit(int unit)
{
    int min = date(unit).time().minute();
    return min == 30;
}

QString HourViewHandler::text(int unit)
{
    return date(unit).toString(QString::fromLatin1("h:00"));
}

Utilities::FastDateTime HourViewHandler::date(int unit, Utilities::FastDateTime reference)
{
    if (reference.isNull())
        reference = m_startDate;
    return reference.addSecs(60 * 10 * unit);
}

QString HourViewHandler::unitText() const
{
    return i18n("10 Minutes");
}

void TenMinuteViewHandler::init(const Utilities::FastDateTime &startDate)
{
    ViewHandler::init(Utilities::FastDateTime(startDate.date(),
                                              QTime(startDate.time().hour(), 10 * (int)floor(startDate.time().minute() / 10.0), 0)));
}

bool TenMinuteViewHandler::isMajorUnit(int unit)
{
    return (date(unit).time().minute() % 10) == 0;
}

bool TenMinuteViewHandler::isMidUnit(int unit)
{
    int min = date(unit).time().minute();
    return (min % 10) == 5;
}

QString TenMinuteViewHandler::text(int unit)
{
    return date(unit).toString(QString::fromLatin1("h:mm"));
}

Utilities::FastDateTime TenMinuteViewHandler::date(int unit, Utilities::FastDateTime reference)
{
    if (reference.isNull())
        reference = m_startDate;
    return reference.addSecs(60 * unit);
}

QString TenMinuteViewHandler::unitText() const
{
    return i18n("1 Minute");
}

void MinuteViewHandler::init(const Utilities::FastDateTime &startDate)
{
    ViewHandler::init(Utilities::FastDateTime(startDate.date(),
                                              QTime(startDate.time().hour(),
                                                    startDate.time().minute(), 0)));
}

bool MinuteViewHandler::isMajorUnit(int unit)
{
    return date(unit).time().second() == 0;
}

bool MinuteViewHandler::isMidUnit(int unit)
{
    int sec = date(unit).time().second();
    return sec == 30;
}

QString MinuteViewHandler::text(int unit)
{
    return date(unit).toString(QString::fromLatin1("h:mm"));
}

Utilities::FastDateTime MinuteViewHandler::date(int unit, Utilities::FastDateTime reference)
{
    if (reference.isNull())
        reference = m_startDate;
    return reference.addSecs(10 * unit);
}

QString MinuteViewHandler::unitText() const
{
    return i18n("10 Seconds");
}

// vi:expandtab:tabstop=4 shiftwidth=4:
