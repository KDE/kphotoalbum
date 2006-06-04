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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "ImageDate.h"
#include <qstringlist.h>
#include <klocale.h>
#include "Settings/SettingsData.h"
#include <qregexp.h>
#include <kdebug.h>

using namespace DB;

ImageDate::ImageDate( const QDate& date )
{
    _start = date;
    _end = date;
}

ImageDate::ImageDate( const QDateTime& date)
{
    _start = date;
    _end = date;
}


bool ImageDate::operator<=( const ImageDate& other ) const
{
    // This operator is used by QMap when checking for equal elements, thus we need the second part too.
    return _start < other._start || (_start == other._start && _end <= other._end);
}

ImageDate::ImageDate()
{
}

bool ImageDate::isNull() const
{
    return _start.isNull();
}

static bool isFirstSecOfMonth( const QDateTime& date )
{
    return date.date().day() == 1 && date.time().hour() == 0 && date.time().minute() == 0;
}

static bool isLastSecOfMonth( QDateTime date )
{
    return isFirstSecOfMonth( date.addSecs(1) );
}

static bool isFirstSecOfDay( const QDateTime& time )
{
    return time.time().hour() == 0 && time.time().minute() == 0 && time.time().second() == 0;
}

static bool isLastSecOfDay( const QDateTime& time )
{
    return time.time().hour() == 23 && time.time().minute() == 59 && time.time().second() == 59;
}

QString ImageDate::toString( bool withTime ) const
{
    if ( _start.isNull() )
        return QString::null;

    if ( _start == _end ) {
        if ( withTime && !isFirstSecOfDay(_start))
            return _start.toString( QString::fromLatin1( "d. MMM yyyy hh:mm:ss" ) );
        else
            return _start.toString( QString::fromLatin1( "d. MMM yyyy" ) );
    }

    // start is different from end.
    if ( isFirstSecOfMonth(_start) && isLastSecOfMonth(_end) ) {
        if ( _start.date().month() == 1 && _end.date().month() == 12 ) {
            if ( _start.date().year() == _end.date().year() ) {
                // 2005
                return QString::number( _start.date().year() );
            }
            else {
                // 2005-2006
                return QString::fromLatin1("%1 - %2").arg( _start.date().year() ).arg( _end.date().year() );
            }
        }
        else {
            // a whole month, but not a whole year.
            if ( _start.date().year() == _end.date().year() &&_start.date().month() == _end.date().month() ) {
                // jan 2005
                return QString::fromLatin1( "%1 %2" ).arg( QDate::shortMonthName(_start.date().month()) ).arg(_start.date().year() );
            }
            else {
                // jan 2005 - feb 2006
                return QString::fromLatin1( "%1 %2 - %3 %4" ).arg( QDate::shortMonthName(_start.date().month()) ).arg(_start.date().year() )
                    .arg( QDate::shortMonthName( _end.date().month() ) ).arg( _end.date().year() );
            }
        }
    }

    if ( isFirstSecOfDay( _start ) && isLastSecOfDay( _end ) ) {
        if (_start.date() == _end.date() ) {
            // A whole day
            return _start.toString( QString::fromLatin1( "d. MMM yyyy" ) );
        }
        else {
            // A day range
            return QString::fromLatin1("%1 - %2" )
                .arg(_start.toString( QString::fromLatin1( "d. MMM yyyy" ) ) )
                .arg(_end.toString( QString::fromLatin1( "d. MMM yyyy" ) ) );
        }
    }


    // Last resort show the whole complete time stamp
    Q_ASSERT( false ); // I can't see how we can get here.
    if ( withTime && ( !isFirstSecOfDay( _start ) || !isLastSecOfDay( _end ) ))
        return QString::fromLatin1("%1 - %2" )
            .arg(_start.toString( QString::fromLatin1( "d. MMM yyyy hh:mm" ) ) )
            .arg(_end.toString( QString::fromLatin1( "d. MMM yyyy hh:mm" ) ) );
    else
        return QString::fromLatin1("%1 - %2" )
            .arg(_start.toString( QString::fromLatin1( "d. MMM yyyy" ) ) )
            .arg(_end.toString( QString::fromLatin1( "d. MMM yyyy" ) ) );
}

bool ImageDate::operator==( const ImageDate& other ) const
{
    return _start == other._start && _end == other._end;
}

bool ImageDate::operator!=( const ImageDate& other )
{
    return !(*this == other );
}

QString ImageDate::formatRegexp()
{
    static QString str;
    if ( str.isNull() ) {
        str = QString::fromLatin1( "^((\\d\\d?)([-. /]+|$))?((" );
        for ( int i = 1; i <= 12; ++i )
            str += QString::fromLatin1("%1|").arg(monthName(i).lower() );
        str += QString::fromLatin1("\\d?\\d)([-. /]+|$))?(\\d\\d(\\d\\d)?)?$" );
    }
    return str;
}

QDateTime ImageDate::start() const
{
    return _start;
}

QDateTime ImageDate::end() const
{
    return _end;
}

bool ImageDate::operator<( const ImageDate& other ) const
{
    return start() < other.start() || start() == other.start() && end() < other.end();
}

QString ImageDate::monthName( int month )
{
    switch ( month ) {
    case 1: return i18n("Jan");
    case 2: return i18n("Feb");
    case 3: return i18n("Mar");
    case 4: return i18n("Apr");
    case 5: return i18n("May");
    case 6: return i18n("Jun");
    case 7: return i18n("Jul");
    case 8: return i18n("Aug");
    case 9: return i18n("Sep");
    case 10: return i18n("Oct");
    case 11: return i18n("Nov");
    case 12: return i18n("Dec");
    }
    qWarning("monthName invoked with invalid name");
    return QString::fromLatin1("");
}

ImageDate::ImageDate( const QDateTime& start, const QDateTime& end )
{
    _start = start;
    _end = end;
}

static QDate addMonth( int year, int month )
{
    if ( month == 12 ) {
        year++;
        month = 1;
    }
    else
        month++;
    return QDate( year, month, 1 );
}

ImageDate::ImageDate( int yearFrom, int monthFrom, int dayFrom, int yearTo, int monthTo, int dayTo, int hourFrom, int minuteFrom, int secondFrom )
{
    if ( yearFrom <= 0 ) {
        _start = QDateTime();
        _end = QDateTime();
        return;
    }

    if ( monthFrom <= 0 ) {
        _start = QDateTime( QDate( yearFrom, 1, 1 ) );
        _end = QDateTime( QDate( yearFrom+1, 1, 1 ) ).addSecs(-1);
    }
    else if ( dayFrom <= 0 ) {
        _start = QDateTime( QDate( yearFrom, monthFrom, 1 ) );
        _end = QDateTime( addMonth( yearFrom, monthFrom ) ).addSecs(-1);
    }
    else if ( hourFrom < 0 ) {
        _start = QDateTime( QDate( yearFrom, monthFrom, dayFrom ) );
        _end = QDateTime( QDate( yearFrom, monthFrom, dayFrom ).addDays(1) ).addSecs(-1);
    }
    else if ( minuteFrom < 0 ) {
        _start = QDateTime( QDate( yearFrom, monthFrom, dayFrom ), QTime( hourFrom, 0, 0 ) );
        _end = QDateTime( QDate( yearFrom, monthFrom, dayFrom ), QTime( hourFrom, 23, 59 ) );
    }
    else if ( secondFrom < 0 ) {
        _start = QDateTime( QDate( yearFrom, monthFrom, dayFrom ), QTime( hourFrom, minuteFrom, 0 ) );
        _end = QDateTime( QDate( yearFrom, monthFrom, dayFrom ), QTime( hourFrom, minuteFrom, 59 ) );
    }
    else {
        _start = QDateTime( QDate( yearFrom, monthFrom, dayFrom ), QTime( hourFrom, minuteFrom, secondFrom ) );
        _end =   _start;
    }

    if ( yearTo > 0 ) {
        _end = QDateTime( QDate( yearTo +1 , 1, 1 ) ).addSecs( -1 );

        if ( monthTo > 0 ) {
            _end = QDateTime( addMonth( yearTo, monthTo ) ).addSecs( -1 );

            if ( dayTo > 0 ) {
                if ( dayFrom == dayTo && monthFrom == monthTo && yearFrom == yearTo )
                    _end = _start;
                else
                    _end = QDateTime( QDate( yearTo, monthTo, dayTo ).addDays(1) ).addSecs(-1);
            }
        }
    }
}

QDate ImageDate::parseDate( const QString& date, bool startDate )
{
    int year = 0;
    int month = 0;
    int day = 0;

    QRegExp regexp( formatRegexp(), false );

    if ( regexp.exactMatch( date ) ) {
        QString dayStr = regexp.cap(2);
        QString monthStr = regexp.cap(5).lower();
        QString yearStr= regexp.cap(7);

        if ( dayStr.length() != 0 )
            day = dayStr.toInt();

        if ( yearStr.length() != 0 ) {
            year = yearStr.toInt();
            if ( year < 50 )
                year += 2000;
            if ( year < 100 )
                year += 1900;
        }
        if ( monthStr.length() != 0 ) {
            if ( monthStr == monthName(1).lower() )
                month = 1;
            else if ( monthStr == monthName(2).lower() )
                month = 2;
            else if ( monthStr == monthName(3).lower() )
                month = 3;
            else if ( monthStr == monthName(4).lower() )
                month = 4;
            else if ( monthStr == monthName(5).lower() )
                month = 5;
            else if ( monthStr == monthName(6).lower() )
                month = 6;
            else if ( monthStr == monthName(7).lower() )
                month = 7;
            else if ( monthStr == monthName(8).lower() )
                month = 8;
            else if ( monthStr == monthName(9).lower() )
                month = 9;
            else if ( monthStr == monthName(10).lower() )
                month = 10;
            else if ( monthStr == monthName(11).lower() )
                month = 11;
            else if ( monthStr == monthName(12).lower() )
                month = 12;
            else
                month = monthStr.toInt();
        }
        if ( year == 0 )
            year = QDate::currentDate().year();
        if ( month == 0 ) {
            if ( startDate ) {
                month = 1;
                day = 1;
            }
            else {
                month = 12;
                day = 31;
            }
        }
        else if ( day == 0 ) {
            if ( startDate )
                day = 1;
            else
                day = QDate( year, month, 1 ).daysInMonth();
        }
        return QDate( year, month, day );
    }
    else
        return QDate();
}

bool ImageDate::hasValidTime() const
{
    return _start == _end;
}

ImageDate::ImageDate( const QDate& start, QDate end, const QTime& time )
{
    if ( end.isNull() )
        end = start;

    if ( start == end && time.isValid() ) {
        _start = QDateTime( start, time );
        _end = _start;
    }
    else {
        _start = QDateTime( start, QTime( 0,0,0 ) );
        _end = QDateTime( end, QTime( 23, 59, 59 ) );
    }
}

ImageDate::MatchType ImageDate::isIncludedIn( const ImageDate& searchRange )
{
    if ( searchRange.start() <= start() && searchRange.end() >= end() )
        return ExactMatch;

    if ( searchRange.start() <= end() && searchRange.end() >= start() ) {
        return RangeMatch;
    }
    return DontMatch;
}

bool ImageDate::includes( const QDateTime& date )
{
    return ImageDate( date ).isIncludedIn( *this ) == ExactMatch;
}

