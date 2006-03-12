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

#ifndef IMAGEDATE_H
#define IMAGEDATE_H
#include <qstring.h>
#include <qdatetime.h>

class ImageDate {
public:
    ImageDate();
    ImageDate( const QDateTime& start, const QDateTime& end );
    ImageDate( int yearFrom, int monthFrom, int dayFrom, int yearTo, int monthTo, int dayTo, int hourFrom, int minuteFrom, int secondFrom );
    ImageDate( const QDate& );
    ImageDate( const QDateTime& );
    ImageDate( const QDate& start, QDate end, const QTime& time );

    QDateTime start() const;
    QDateTime end() const;
    static QDate parseDate( const QString& date, bool startDate );

    bool operator<( const ImageDate& other ) const;
    bool operator<=( const ImageDate& other ) const;
    bool operator==( const ImageDate& other ) const;
    bool operator!=( const ImageDate& other );

    bool isValid() const { return !isNull(); }
    bool isNull() const;
    QString toString( bool withTime = true ) const;
    bool hasValidTime() const;

    enum MatchType { DontMatch, ExactMatch, RangeMatch };
    MatchType isIncludedIn( const ImageDate& searchRange );
    bool includes( const QDateTime& date );

protected:
    static QString monthName( int month );
    static QString formatRegexp();

private:
    QDateTime _start, _end;
};

#endif /* IMAGEDATE_H */

