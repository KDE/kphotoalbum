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

#ifndef IMAGEDATE_H
#define IMAGEDATE_H
#include <qstring.h>
#include <qdatetime.h>

class ImageDate {
public:
    ImageDate();
    ImageDate( int day, int month, int year );
    ImageDate( const QDate& );
    int year() const;
    int month() const;
    int day() const;
    void setDate( const QDate& );
    void setYear( int );
    void setMonth( int );
    void setDay( int );
    bool operator<=( const ImageDate& other ) const;
    bool isNull() const;
    QString toString();
    operator QString() { return toString(); }
    bool operator==( const ImageDate& other );
    bool operator!=( const ImageDate& other );

private:
    int _year, _month, _day;
};

#endif /* IMAGEDATE_H */

