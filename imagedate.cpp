/*
 *  Copyright (c) 2003-2004 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

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
}

ImageDate::ImageDate( const QDate& date )
{
    setDate( date );
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
}

void ImageDate::setMonth( int month )
{
    _month = month;
}

void ImageDate::setDay( int day )
{
    _day = day;
}

void ImageDate::setHour( int hour )
{
    _hour = hour;
}

void ImageDate::setMinute( int minute )
{
    _minute = minute;
}

void ImageDate::setSecond( int second )
{
    _second = second;
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

    QStringList month;
    month << i18n("Jan") << i18n("Feb") << i18n("Mar") << i18n("Apr") << i18n("May") << i18n("Jun") << i18n("Jul") << i18n("Aug")
          << i18n("Sep") << i18n("Oct") << i18n("Nov") << i18n("Dec");

    if ( _day > 0 && _month > 0 )
        result = QString::fromLatin1("%1. %2").arg(_day).arg(month[_month-1]);
    else if ( _day > 0 && _month <= 0 )
        result = QString::fromLatin1("%1/???").arg(_day);
    else if ( _day <= 0 && _month > 0 )  {
        result = month[_month-1];
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

bool ImageDate::operator==( const ImageDate& other )
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

}

QTime ImageDate::getTime()
{
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
    QDate date = QDate::currentDate();
    int day = 1;
    int month = 1;
    int year = 1970;
    if ( _day > 0 )
        day = _day;
    if ( _month > 0 )
        _month = _month;
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
            if ( month == QString::fromLatin1( "jan" ) )
                _month = 1;
            else if ( month == QString::fromLatin1( "feb" ) )
                _month = 2;
            else if ( month == QString::fromLatin1( "mar" ) )
                _month = 3;
            else if ( month == QString::fromLatin1( "apr" ) )
                _month = 4;
            else if ( month == QString::fromLatin1( "may" ) )
                _month = 5;
            else if ( month == QString::fromLatin1( "jun" ) )
                _month = 6;
            else if ( month == QString::fromLatin1( "jul" ) )
                _month = 7;
            else if ( month == QString::fromLatin1( "aug" ) )
                _month = 8;
            else if ( month == QString::fromLatin1( "sep" ) )
                _month = 9;
            else if ( month == QString::fromLatin1( "oct" ) )
                _month = 10;
            else if ( month == QString::fromLatin1( "nov" ) )
                _month = 11;
            else if ( month == QString::fromLatin1( "dec" ) )
                _month = 12;
            else
                _month = month.toInt();
        }
    }
}

QString ImageDate::formatRegexp()
{
    return QString::fromLatin1( "^((\\d\\d?)([-. /]+|$))?((jan|feb|mar|apr|may|jun|jul|aug|sep|nov|dec|\\d?\\d)([-. /]+|$))?(\\d\\d(\\d\\d)?)?$" );
}
