#include "imagedate.h"
#include <qstringlist.h>

ImageDate::ImageDate( int day, int month, int year )
{
    _day = day;
    _month = month;
    _year = year;
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

bool ImageDate::operator<=( ImageDate& other )
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

ImageDate::ImageDate() :_year(0), _month(0), _day(0)
{
}

bool ImageDate::isNull() const
{
    return ( _year == 0 && _month == 0 && _day == 0 );
}

QString ImageDate::toString()
{
    QString result;

    QStringList month;
    month << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun" << "Jul" << "Aug"
          << "Sep" << "Oct" << "Nov" << "Dec";

    if ( _day != 0 && _month != 0 )
        result = QString("%1. %2").arg(_day).arg(month[_month-1]);
    else if ( _day != 0 && _month == 0 )
        result = QString("%1/???").arg(_day);
    else if ( _day == 0 && _month != 0 )  {
        result = month[_month-1];
    }

    if ( !result.isEmpty() && _year != 0 )
        result += " ";

    if ( _year != 0 )
        result += QString::number( _year );

    return result;
}

bool ImageDate::operator==( const ImageDate& other )
{
    return
        ( _year == other._year &&
          _month == other._month &&
          _day == other._day );
}

bool ImageDate::operator!=( const ImageDate& other )
{
    return !(*this == other );
}
