/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "dateviewhandler.h"
#include <qobject.h>
#include <math.h>
#include <klocale.h>
#include <kglobal.h>
#include <kapplication.h>
#include "util.h"

void DateViewHandler::init( const QDateTime& startDate )
{
    _startDate = startDate;
}

bool DateViewHandler::isMidUnit( int /*unit*/ )
{
    return false;
}








void DecadeViewHandler::init( const QDateTime& startDate )
{
    QDateTime date = QDateTime( QDate( startDate.date().year(), 1, 1 ), QTime( 0,0,0 ) );
    DateViewHandler::init( date );
}

bool DecadeViewHandler::isMajorUnit( int unit )
{
    return date(unit).date().year() % 10 == 0;
}

bool DecadeViewHandler::isMidUnit( int unit )
{
    return date(unit).date().year() % 5 == 0;
}

QString DecadeViewHandler::text( int unit )
{
    return QString::number( date( unit ).date().year() );
}

QDateTime DecadeViewHandler::date(int unit, QDateTime reference )
{
    if ( reference.isNull() ) reference = _startDate;
    return reference.addMonths( 12*unit );
}


QString DecadeViewHandler::unitText() const
{
    return i18n( "1 Year" );
}

void YearViewHandler::init( const QDateTime& startDate )
{
    QDateTime date = QDateTime( QDate( startDate.date().year(), startDate.date().month(), 1 ), QTime( 0,0,0 ) );
    DateViewHandler::init( date );
}

bool YearViewHandler::isMajorUnit( int unit )
{
    return date(unit).date().month() == 1;
}

bool YearViewHandler::isMidUnit( int unit )
{
    return date( unit ).date().month() == 7;
}

QString YearViewHandler::text( int unit )
{
    return QString::number( date( unit ).date().year() );
}

QDateTime YearViewHandler::date(int unit, QDateTime reference )
{
    if ( reference.isNull() ) reference = _startDate;
    return reference.addMonths( unit );
}

QString YearViewHandler::unitText() const
{
    return i18n("1 Month");
}





void MonthViewHandler::init( const QDateTime& startDate)
{
    QDate date = startDate.date().addDays( - startDate.date().dayOfWeek() +1 ); // Wind to monday
    DateViewHandler::init( QDateTime( date, QTime( 0, 0, 0 ) ) );
}

bool MonthViewHandler::isMajorUnit( int unit )
{
    return date(unit).date().day() <= 7;
}

QString MonthViewHandler::text( int unit )
{
    static int lastunit=99999;
    static int printedLast = false;
    if ( unit < lastunit )
        printedLast = true;
    QString str;
    if ( !printedLast )
        str=KGlobal::locale()->formatDate( date(unit).date(), true );
    printedLast = !printedLast;
    lastunit = unit;
    return str;
}

QDateTime MonthViewHandler::date(int unit, QDateTime reference )
{
    if ( reference.isNull() ) reference = _startDate;
    return reference.addDays( 7*unit );
}

QString MonthViewHandler::unitText() const
{
    return i18n("1 Week");
}



void WeekViewHandler::init( const QDateTime& startDate)
{
    DateViewHandler::init( QDateTime( startDate.date(), QTime( 0, 0, 0 ) ) );
}

bool WeekViewHandler::isMajorUnit( int unit )
{
    return date(unit).date().dayOfWeek() == 1;
}

QString WeekViewHandler::text( int unit )
{
    return KGlobal::locale()->formatDate(date(unit).date(), true);
}

QDateTime WeekViewHandler::date(int unit, QDateTime reference )
{
    if ( reference.isNull() ) reference = _startDate;
    return reference.addDays( unit );
}

QString WeekViewHandler::unitText() const
{
    return i18n("1 Day");
}




void DayViewHandler::init( const QDateTime& startDate)
{
    QDateTime date = startDate;
    if ( date.time().hour() %2 )
        date = date.addSecs( 60*60 );

    DateViewHandler::init( QDateTime( date.date(), QTime( date.time().hour(), 0, 0 ) ) );
}

bool DayViewHandler::isMajorUnit( int unit )
{
    int h = date(unit).time().hour();
    return h == 0 || h == 12;
}

bool DayViewHandler::isMidUnit( int unit )
{
    int h = date(unit).time().hour();
    return h == 6 || h == 18;
}

QString DayViewHandler::text( int unit )
{
    if (  date(unit).time().hour() == 0 )
        return KGlobal::locale()->formatDate(date(unit).date(), true);
    else
        return date(unit).toString( QString::fromLatin1( "h:00" ) );
}

QDateTime DayViewHandler::date(int unit, QDateTime reference )
{
    if ( reference.isNull() ) reference = _startDate;
    return reference.addSecs( 2*60*60*unit );
}

QString DayViewHandler::unitText() const
{
    return i18n("2 Hours");
}




void HourViewHandler::init( const QDateTime& startDate)
{
    DateViewHandler::init( QDateTime( startDate.date(),
                                      QTime( startDate.time().hour(), 10 * (int) floor(startDate.time().minute()/10.0), 0 ) ) );
}

bool HourViewHandler::isMajorUnit( int unit )
{
    return date(unit).time().minute() == 0;
}

bool HourViewHandler::isMidUnit( int unit )
{
    int min = date(unit).time().minute();
    return min == 30;
}

QString HourViewHandler::text( int unit )
{
    return date(unit).toString( QString::fromLatin1( "h:00" ) );
}

QDateTime HourViewHandler::date(int unit, QDateTime reference )
{
    if ( reference.isNull() ) reference = _startDate;
    return reference.addSecs( 60 * 10 * unit );
}

QString HourViewHandler::unitText() const
{
    return i18n("10 Minutes");
}

