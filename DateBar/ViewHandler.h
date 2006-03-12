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

#ifndef DATEVIEWHANDLER_H
#define DATEVIEWHANDLER_H
#include <qdatetime.h>
#include <qstring.h>
#include "imagedatecollection.h"

namespace DateBar
{

class ViewHandler
{
public:
    virtual void init( const QDateTime& startDate ) = 0;
    virtual bool isMajorUnit( int unit ) = 0;
    virtual bool isMidUnit( int unit );
    virtual QString text( int unit ) = 0;
    virtual QDateTime date(int unit, QDateTime reference = QDateTime() ) = 0;
    virtual QString unitText() const = 0;

protected:
    QDateTime _startDate;
};


class DecadeViewHandler :public ViewHandler
{
public:
    virtual void init( const QDateTime& startDate );
    virtual bool isMajorUnit( int unit );
    virtual bool isMidUnit( int unit );
    virtual QString text( int unit );
    virtual QDateTime date(int unit, QDateTime reference = QDateTime() );
    virtual QString unitText() const;
};

class YearViewHandler :public ViewHandler
{
public:
    virtual void init( const QDateTime& startDate );
    virtual bool isMajorUnit( int unit );
    virtual bool isMidUnit( int unit );
    virtual QString text( int unit );
    virtual QDateTime date(int unit, QDateTime reference = QDateTime() );
    virtual QString unitText() const;
};

class MonthViewHandler :public ViewHandler
{
public:
    virtual void init( const QDateTime& startDate );
    virtual bool isMajorUnit( int unit );
    virtual QString text( int unit );
    virtual QDateTime date(int unit, QDateTime reference = QDateTime() );
    virtual QString unitText() const;
};

class WeekViewHandler :public ViewHandler
{
public:
    virtual void init( const QDateTime& startDate );
    virtual bool isMajorUnit( int unit );
    virtual QString text( int unit );
    virtual QDateTime date(int unit, QDateTime reference = QDateTime() );
    virtual QString unitText() const;
};

class DayViewHandler :public ViewHandler
{
public:
    virtual void init( const QDateTime& startDate );
    virtual bool isMajorUnit( int unit );
    virtual bool isMidUnit( int unit );
    virtual QString text( int unit );
    virtual QDateTime date(int unit, QDateTime reference = QDateTime() );
    virtual QString unitText() const;
};

class HourViewHandler :public ViewHandler
{
public:
    virtual void init( const QDateTime& startDate );
    virtual bool isMajorUnit( int unit );
    virtual bool isMidUnit( int unit );
    virtual QString text( int unit );
    virtual QDateTime date(int unit, QDateTime reference = QDateTime() );
    virtual QString unitText() const;
};

}


#endif /* DATEVIEWHANDLER_H */

