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

#include "imagedate.h"
#include <qstringlist.h>
#include <klocale.h>
#include "options.h"
#include <qregexp.h>

ImageDate::ImageDate( int day, int month, int year )
{
    _day = day;
    _month = month;
    _year = year;
    _hour = -1;
    _minute = -1;
    _second = -1;
    _dirty = true;
}

ImageDate::ImageDate( const QDate& date )
{
    setDate( date );
    _hour = -1;
    _minute = -1;
    _second = -1;
    _dirty = true;
}

ImageDate::ImageDate( const QDateTime& date)
{
    setDate( date.date() );
    setTime( date.time() );
    Q_ASSERT( date.time().isValid() );
    _dirty = true;
}

int ImageDate::year() const
{
    return _year;
}

int ImageDate::month() const
{
    return _month;
}

int ImageDate::day() const
{
    return _day;
}

int ImageDate::hour() const
{
    return _hour;
}

int ImageDate::minute() const
{
    return _minute;
}

int ImageDate::second() const
{
    return _second;
}

void ImageDate::setYear( int year )
{
    _year = year;
    _dirty = true;
}

void ImageDate::setMonth( int month )
{
    _month = month;
    _dirty = true;
}

void ImageDate::setDay( int day )
{
    _day = day;
    _dirty = true;
}

void ImageDate::setHour( int hour )
{
    Q_ASSERT( hour >= -1 && hour <= 24 );
    _hour = hour;
    _dirty = true;
}

void ImageDate::setMinute( int minute )
{
    Q_ASSERT( minute >= -1 && minute <= 59 );
    _minute = minute;
    _dirty = true;
}

void ImageDate::setSecond( int second )
{
    _second = second;
    _dirty = true;
}


bool ImageDate::operator<=( const ImageDate& other ) const
{
    bool ignoreYear =  ( _year == 0 || other._year == 0 );
    bool ignoreMonth = ( _month == 0 || other._month == 0 );
    bool ignoreDay = (_day == 0 || other._day == 0 );
    bool yearEqual = ( ignoreYear || _year == other._year );
    bool monthEqual = ( ignoreMonth ||  _month == other._month );

    if ( !ignoreYear && _year > other._year )
        return false;
    else if ( yearEqual && !ignoreMonth && _month > other._month )
        return false;
    else if ( yearEqual && monthEqual && !ignoreDay && _day > other._day )
        return false;
    else
        return true;
}

ImageDate::ImageDate() :_year(0), _month(0), _day(0), _hour(-1), _minute(-1), _second(-1)
{
}

bool ImageDate::isNull() const
{
    return ( _year <= 0 && _month <= 0 && _day <= 0);
}

QString ImageDate::toString( bool withTime ) const
{
    QString result;

    if ( _day > 0 && _month > 0 )
        result = QString::fromLatin1("%1. %2").arg(_day).arg(monthName(_month));
    else if ( _day > 0 && _month <= 0 )
        result = QString::fromLatin1("%1/???").arg(_day);
    else if ( _day <= 0 && _month > 0 )  {
        result = monthName(_month);
    }

    if ( !result.isEmpty() && _year != 0 )
        result += QString::fromLatin1(" ");

    if ( _year != 0 )
        result += QString::number( _year );

    if ( withTime && _hour>=0 && _minute>=0 && _second>=0 && Options::instance()->showTime() &&
         ( _hour != 0 || _minute != 0 || _second != 0 ) ) {
        if (_hour<=9)
            result += QString::fromLatin1(" 0%1:").arg(_hour);
        else
            result += QString::fromLatin1(" %1:").arg(_hour);

        if (_minute<=9)
            result += QString::fromLatin1("0%1:").arg(_minute);
        else
            result += QString::fromLatin1("%1:").arg(_minute);

        if (_second<=9)
            result += QString::fromLatin1("0%1").arg(_second);
        else
            result += QString::fromLatin1("%1").arg(_second);
    }
    return result;
}

bool ImageDate::operator==( const ImageDate& other ) const
{
    return
        ( _year == other._year &&
          _month == other._month &&
          _day == other._day &&
          _hour == other._hour &&
          _minute == other._minute &&
          _second == other._second );
}

bool ImageDate::operator!=( const ImageDate& other )
{
    return !(*this == other );
}

void ImageDate::setDate( const QDate& date )
{
    _day = date.day();
    _month = date.month();
    _year = date.year();
}

void ImageDate::setTime( const QTime& time )
{
    _hour = time.hour();
    _minute = time.minute();
    _second = time.second();
    _dirty = true;
}

QTime ImageDate::getTime()
{
    // PENDING(blackie) Do I need this anymore? Replace with min() or max()
    if ( _hour == -1 || _minute == -1 || _second == -1 )
        return QTime();
    else
        return QTime(_hour,_minute,_second);
}

bool ImageDate::hasValidTime() const
{
    return QTime::isValid( _hour, _minute, _second );
}

QDate ImageDate::getDate()
{
    // PENDING(blackie) Do I need this anymore? Replace with min() or max()
    int day = 1;
    int month = 1;
    int year = 1970;
    if ( _day > 0 )
        day = _day;
    if ( _month > 0 )
        month = _month;
    if ( _year > 0 )
        year = _year;
    return QDate( year, month, day );
}

void ImageDate::setDate( const QString& date )
{
    _year = 0;
    _month = 0;
    _day = 0;

    QRegExp regexp( formatRegexp(), false );

    if ( regexp.exactMatch( date ) ) {
        QString day = regexp.cap(2);
        QString month = regexp.cap(5).lower();
        QString year= regexp.cap(7);

        if ( day.length() != 0 )
            _day = day.toInt();

        if ( year.length() != 0 ) {
            _year = year.toInt();
            if ( _year < 50 )
                _year += 2000;
            if ( _year < 100 )
                _year += 1900;
        }
        if ( month.length() != 0 ) {
            if ( month == monthName(1).lower() )
                _month = 1;
            else if ( month == monthName(2).lower() )
                _month = 2;
            else if ( month == monthName(3).lower() )
                _month = 3;
            else if ( month == monthName(4).lower() )
                _month = 4;
            else if ( month == monthName(5).lower() )
                _month = 5;
            else if ( month == monthName(6).lower() )
                _month = 6;
            else if ( month == monthName(7).lower() )
                _month = 7;
            else if ( month == monthName(8).lower() )
                _month = 8;
            else if ( month == monthName(9).lower() )
                _month = 9;
            else if ( month == monthName(10).lower() )
                _month = 10;
            else if ( month == monthName(11).lower() )
                _month = 11;
            else if ( month == monthName(12).lower() )
                _month = 12;
            else
                _month = month.toInt();
        }
    }

    _dirty = true;
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

bool ImageDate::isFuzzyData()
{
    return _year == 0 || _month == 0 || _day == 0;
}

QDateTime ImageDate::min() const
{
    if ( _dirty )
        calcMinMax();

    return _min;
}

QDateTime ImageDate::max() const
{
    if ( _dirty )
        calcMinMax();

    return _max;
}

void ImageDate::calcMinMax() const
{
    Q_ASSERT( _hour >= -1 && _hour <= 24 );

    _min = QDateTime( QDate( _year, _month == 0 ? 1 : _month, _day == 0 ? 1 : _day ),
                      QTime( _hour == -1 ? 0 : _hour, _minute == -1 ? 0 : _minute, _second == -1 ? 0 : _second ) );


    int year = ( _year == 0 ? 3000 : _year );
    int month = (_month == 0 ? 12 : _month );
    int day;

    if ( _day == 0 )
        day = QDate( year, month, 1 ).daysInMonth();
    else
        day = _day;

    _max = QDateTime( QDate( year, month, day ),
                      QTime( _hour == -1 ? 23 : _hour, _minute == -1 ? 59 : _minute, _second == -1 ? 59 : _second ) );
    _dirty = false;
}

bool ImageDate::operator<( const ImageDate& other ) const
{
    return min() < other.min();
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

