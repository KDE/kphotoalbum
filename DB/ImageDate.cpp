/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "ImageDate.h"
#include <KLocalizedString>
#include <QLocale>
#include <qregexp.h>

using namespace DB;

ImageDate::ImageDate( const QDate& date )
{
    m_start = QDateTime( date, QTime(0,0,0) );
    m_end = QDateTime( date, QTime(0,0,0) );
}

ImageDate::ImageDate( const QDateTime& date)
{
    m_start = date;
    m_end = date;
}


bool ImageDate::operator<=( const ImageDate& other ) const
{
    // This operator is used by QMap when checking for equal elements, thus we need the second part too.
    return m_start < other.m_start || (m_start == other.m_start && m_end <= other.m_end);
}

ImageDate::ImageDate()
{
}

bool ImageDate::isNull() const
{
    return m_start.isNull();
}

bool ImageDate::isFuzzy() const
{
    return m_start != m_end;
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
    if ( m_start.isNull() )
        return QString();

    if ( m_start == m_end ) {
        if ( withTime && !isFirstSecOfDay(m_start))
            return m_start.toString( QString::fromLatin1( "d. MMM yyyy hh:mm:ss" ) );
        else
            return m_start.toString( QString::fromLatin1( "d. MMM yyyy" ) );
    }

    // start is different from end.
    if ( isFirstSecOfMonth(m_start) && isLastSecOfMonth(m_end) ) {
        if ( m_start.date().month() == 1 && m_end.date().month() == 12 ) {
            if ( m_start.date().year() == m_end.date().year() ) {
                // 2005
                return QString::number( m_start.date().year() );
            }
            else {
                // 2005-2006
                return QString::fromLatin1("%1 - %2").arg( m_start.date().year() ).arg( m_end.date().year() );
            }
        }
        else {
            // a whole month, but not a whole year.
            if ( m_start.date().year() == m_end.date().year() &&m_start.date().month() == m_end.date().month() ) {
                // jan 2005
                return QString::fromLatin1( "%1 %2" )
                        .arg( QLocale().standaloneMonthName(m_start.date().month(), QLocale::ShortFormat) ).arg(m_start.date().year() );
            }
            else {
                // jan 2005 - feb 2006
                return QString::fromLatin1( "%1 %2 - %3 %4" )
                        .arg( QLocale().standaloneMonthName(m_start.date().month(), QLocale::ShortFormat) ).arg(m_start.date().year() )
                        .arg( QLocale().standaloneMonthName(m_end.date().month(), QLocale::ShortFormat) ).arg( m_end.date().year() );
            }
        }
    }

    if ( isFirstSecOfDay( m_start ) && isLastSecOfDay( m_end ) ) {
        if (m_start.date() == m_end.date() ) {
            // A whole day
            return m_start.toString( QString::fromLatin1( "d. MMM yyyy" ) );
        }
        else {
            // A day range
            return QString::fromLatin1("%1 - %2" )
                .arg(m_start.toString( QString::fromLatin1( "d. MMM yyyy" ) ) )
                .arg(m_end.toString( QString::fromLatin1( "d. MMM yyyy" ) ) );
        }
    }


    // Range smaller than one day.
    if ( withTime && ( !isFirstSecOfDay( m_start ) || !isLastSecOfDay( m_end ) ))
        return QString::fromLatin1("%1 - %2" )
            .arg(m_start.toString( QString::fromLatin1( "d. MMM yyyy hh:mm" ) ) )
            .arg(m_end.toString( QString::fromLatin1( "d. MMM yyyy hh:mm" ) ) );
    else
        return QString::fromLatin1("%1 - %2" )
            .arg(m_start.toString( QString::fromLatin1( "d. MMM yyyy" ) ) )
            .arg(m_end.toString( QString::fromLatin1( "d. MMM yyyy" ) ) );
}

bool ImageDate::operator==( const ImageDate& other ) const
{
    return m_start == other.m_start && m_end == other.m_end;
}

bool ImageDate::operator!=( const ImageDate& other ) const
{
    return !(*this == other );
}

QString ImageDate::formatRegexp()
{
    static QString str;
    if ( str.isEmpty() ) {
        str = QString::fromLatin1( "^((\\d\\d?)([-. /]+|$))?((" );
        QStringList months = monthNames();
        for( QStringList::ConstIterator monthIt = months.constBegin(); monthIt != months.constEnd(); ++monthIt )
            str += QString::fromLatin1("%1|").arg( *monthIt );

        str += QString::fromLatin1("\\d?\\d)([-. /]+|$))?(\\d\\d(\\d\\d)?)?$" );
    }
    return str;
}

QDateTime ImageDate::start() const
{
    return m_start;
}

QDateTime ImageDate::end() const
{
    return m_end;
}

bool ImageDate::operator<( const ImageDate& other ) const
{
    return start() < other.start() || ( start() == other.start() && end() < other.end() );
}


ImageDate::ImageDate( const QDateTime& start, const QDateTime& end )
{
    m_start = start;
    m_end = end;
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
        m_start = QDateTime();
        m_end = QDateTime();
        return;
    }

    if ( monthFrom <= 0 ) {
        m_start = QDateTime( QDate( yearFrom, 1, 1 ) );
        m_end = QDateTime( QDate( yearFrom+1, 1, 1 ) ).addSecs(-1);
    }
    else if ( dayFrom <= 0 ) {
        m_start = QDateTime( QDate( yearFrom, monthFrom, 1 ) );
        m_end = QDateTime( addMonth( yearFrom, monthFrom ) ).addSecs(-1);
    }
    else if ( hourFrom < 0 ) {
        m_start = QDateTime( QDate( yearFrom, monthFrom, dayFrom ) );
        m_end = QDateTime( QDate( yearFrom, monthFrom, dayFrom ).addDays(1) ).addSecs(-1);
    }
    else if ( minuteFrom < 0 ) {
        m_start = QDateTime( QDate( yearFrom, monthFrom, dayFrom ), QTime( hourFrom, 0, 0 ) );
        m_end = QDateTime( QDate( yearFrom, monthFrom, dayFrom ), QTime( hourFrom, 23, 59 ) );
    }
    else if ( secondFrom < 0 ) {
        m_start = QDateTime( QDate( yearFrom, monthFrom, dayFrom ), QTime( hourFrom, minuteFrom, 0 ) );
        m_end = QDateTime( QDate( yearFrom, monthFrom, dayFrom ), QTime( hourFrom, minuteFrom, 59 ) );
    }
    else {
        m_start = QDateTime( QDate( yearFrom, monthFrom, dayFrom ), QTime( hourFrom, minuteFrom, secondFrom ) );
        m_end =   m_start;
    }

    if ( yearTo > 0 ) {
        m_end = QDateTime( QDate( yearTo +1 , 1, 1 ) ).addSecs( -1 );

        if ( monthTo > 0 ) {
            m_end = QDateTime( addMonth( yearTo, monthTo ) ).addSecs( -1 );

            if ( dayTo > 0 ) {
                if ( dayFrom == dayTo && monthFrom == monthTo && yearFrom == yearTo )
                    m_end = m_start;
                else
                    m_end = QDateTime( QDate( yearTo, monthTo, dayTo ).addDays(1) ).addSecs(-1);
            }
        }
    }
}

QDate ImageDate::parseDate( const QString& date, bool startDate )
{
    int year = 0;
    int month = 0;
    int day = 0;

    QRegExp regexp( formatRegexp(), Qt::CaseInsensitive );

    if ( regexp.exactMatch( date ) ) {
        QString dayStr = regexp.cap(2);
        QString monthStr = regexp.cap(5).toLower();
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
            int index = monthNames().indexOf( monthStr );
            if ( index != -1 )
                month = (index%12)+1;
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
    return m_start == m_end;
}

ImageDate::ImageDate( const QDate& start, QDate end, const QTime& time )
{
    if ( end.isNull() )
        end = start;

    if ( start == end && time.isValid() ) {
        m_start = QDateTime( start, time );
        m_end = m_start;
    }
    else {
        m_start = QDateTime( start, QTime( 0,0,0 ) );
        m_end = QDateTime( end, QTime( 23, 59, 59 ) );
    }
}

ImageDate::MatchType ImageDate::isIncludedIn( const ImageDate& searchRange ) const
{
    if ( searchRange.start() <= start() && searchRange.end() >= end() )
        return ExactMatch;

    if ( searchRange.start() <= end() && searchRange.end() >= start() ) {
        return RangeMatch;
    }
    return DontMatch;
}

bool ImageDate::includes( const QDateTime& date ) const
{
    return ImageDate( date ).isIncludedIn( *this ) == ExactMatch;
}

QStringList DB::ImageDate::monthNames()
{
    static QStringList res;
    if ( res.isEmpty() ) {
        for ( int i = 1; i <= 12; ++i ) {
            res << QLocale().standaloneMonthName(i, QLocale::ShortFormat);
        }
        for ( int i = 1; i <= 12; ++i ) {
            res << QLocale().standaloneMonthName(i, QLocale::LongFormat);
        }

        res << i18nc("Abbreviated month name","jan") << i18nc("Abbreviated month name","feb")
            << i18nc("Abbreviated month name","mar") << i18nc("Abbreviated month name","apr")
            << i18nc("Abbreviated month name","may") << i18nc("Abbreviated month name","jun")
            << i18nc("Abbreviated month name","jul") << i18nc("Abbreviated month name","aug")
            << i18nc("Abbreviated month name","sep") << i18nc("Abbreviated month name","oct")
            << i18nc("Abbreviated month name","nov") << i18nc("Abbreviated month name","dec");
        res << QString::fromLatin1("jan") << QString::fromLatin1("feb") << QString::fromLatin1("mar") << QString::fromLatin1("apr")
            << QString::fromLatin1("may") << QString::fromLatin1("jun") << QString::fromLatin1("jul") << QString::fromLatin1("aug")
            << QString::fromLatin1("sep") << QString::fromLatin1("oct") << QString::fromLatin1("nov") << QString::fromLatin1("dec");

        for ( int i = 1; i <= 12; ++i ) {
            res << QLocale().monthName(i, QLocale::ShortFormat);
        }
        for ( int i = 1; i <= 12; ++i ) {
            res << QLocale().monthName(i, QLocale::LongFormat);
        }

        for ( QStringList::iterator it = res.begin(); it != res.end(); ++it )
            *it = it->toLower();
    }
    return res;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
