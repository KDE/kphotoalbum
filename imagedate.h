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

#ifndef IMAGEDATE_H
#define IMAGEDATE_H
#include <qstring.h>
#include <qdatetime.h>

class ImageDate {
public:
    ImageDate();
    ImageDate( int day, int month, int year );
    ImageDate( const QDate& );
    ImageDate( const QDateTime& );

    int year() const;
    int month() const;
    int day() const;

    int hour() const;
    int minute() const;
    int second() const;

    void setDate( const QDate& );
    void setTime( const QTime& );

    QTime getTime();
    QDate getDate();
    void setDate( const QString& date );
    bool isFuzzyData();
    QDateTime min() const;
    QDateTime max() const;

    void setYear( int );
    void setMonth( int );
    void setDay( int );

    void setHour( int );
    void setMinute( int );
    void setSecond( int );

    bool operator<( const ImageDate& other ) const;
    bool operator<=( const ImageDate& other ) const;
    bool isValid() const { return !isNull(); }
    bool isNull() const;
    QString toString( bool withTime = true ) const;
    operator QString() { return toString(); }
    bool operator==( const ImageDate& other ) const;
    bool operator!=( const ImageDate& other );
    bool hasValidTime() const;

    static QString formatRegexp();

protected:
    void calcMinMax() const;
    static QString monthName( int month );

private:
    int _year, _month, _day, _hour, _minute, _second;
    mutable QDateTime _min, _max;
    mutable bool _dirty;
};

#endif /* IMAGEDATE_H */

