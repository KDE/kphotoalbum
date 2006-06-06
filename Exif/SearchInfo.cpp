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

#include "Exif/SearchInfo.h"
#include "Exif/Database.h"
#include <qsqlquery.h>
#include "SearchInfo.h"

/**
 * \class Exif::SearchInfo
 * This class represents a search for Exif information. It is similar in functionality for category searches which is in the
 * class \ref DB::ImageSearchInfo.
 *
 * The search is build, from \ref Exif::SearchDialog, using the functions addRangeKey(), addSearchKey(), and addCamara().
 * The search is stored in an instance of \ref DB::ImageSearchInfo, and may later be executed using search().
 * Once a search has been executed, the application may ask if a given image is in the search result using matches()
 */
void Exif::SearchInfo::addSearchKey( const QString& key, const QValueList<int> values )
{
    _intKeys.append( qMakePair( key, values ) );
}


QStringList Exif::SearchInfo::buildIntKeyQuery() const
{
    QStringList andArgs;
    for( QValueList< QPair<QString, QValueList<int> > >::ConstIterator intIt = _intKeys.begin(); intIt != _intKeys.end(); ++intIt ) {
        QStringList orArgs;
        QString key = (*intIt).first;
        QValueList<int> values =(*intIt).second;

        for( QValueList<int>::Iterator argIt = values.begin(); argIt != values.end(); ++argIt ) {
            orArgs << QString::fromLatin1( "(%1 == %2)" ).arg( key ).arg( *argIt );
        }
        if ( orArgs.count() != 0 )
            andArgs << QString::fromLatin1( "(%1)").arg( orArgs.join( QString::fromLatin1( " or " ) ) );
    }

    return andArgs;
}

void Exif::SearchInfo::addRangeKey( const Range& range )
{
    _rangeKeys.append( range);
}

Exif::SearchInfo::Range::Range( const QString& key )
    :isLowerMin(false), isLowerMax(false), isUpperMin(false), isUpperMax(false), key(key)
{
}

QString Exif::SearchInfo::buildQuery() const
{
    QStringList subQueries;
    subQueries += buildIntKeyQuery();
    subQueries += buildRangeQuery();
    QString cameraQuery = buildCameraSearchQuery();
    if ( !cameraQuery.isNull() )
        subQueries.append( cameraQuery );

    if ( subQueries.empty() )
        return QString::null;
    else
        return  QString::fromLatin1( "SELECT filename from exif WHERE %1" )
            .arg( subQueries.join( QString::fromLatin1( " and " ) ) );
}

QStringList Exif::SearchInfo::buildRangeQuery() const
{
    QStringList result;
    for( QValueList<Range>::ConstIterator it = _rangeKeys.begin(); it != _rangeKeys.end(); ++it ) {
        QString str = sqlForOneRangeItem( *it );
        if ( !str.isNull() )
            result.append( str );
    }
    return result;
}


QString Exif::SearchInfo::sqlForOneRangeItem( const Range& range ) const
{
    // Notice I multiplied factors on each value to ensure that we do not fail due to rounding errors for say 1/3

    if ( range.isLowerMin ) {
        //  Min to Min  means < x
        if ( range.isUpperMin )
            return QString::fromLatin1( "%1 < %2 and %3 > 0" ).arg( range.key ).arg( range.min *1.01 ).arg( range.key );

        //  Min to Max means all images
        if ( range.isUpperMax )
            return QString::null;

        //  Min to y   means <= y
        return QString::fromLatin1( "%1 <= %2 and %3 > 0" ).arg( range.key ).arg( range.max * 1.01 ).arg( range.key );
    }

    //  MAX to MAX   means >= y
    if ( range.isLowerMax )
        return QString::fromLatin1( "%1 > %2" ).arg( range.key ).arg( range.max*0.99 );

    //  x to Max   means >= x
    if ( range.isUpperMax )
        return QString::fromLatin1( "%1 >= %2" ).arg( range.key ).arg( range.min *0.99 );

    //  x to y     means >=x and <=y
    return QString::fromLatin1( "(%1 <= %2 and %3 <= %4)" )
        .arg( range.min * 0.99 )
        .arg( range.key ).arg( range.key )
        .arg( range.max * 1.01);
}

void Exif::SearchInfo::search() const
{
    QString queryStr = buildQuery();
    _emptyQuery = queryStr.isNull();

    // ensure to do SQL queries as little as possible.
    static QString lastQuery = QString::null;
    if ( queryStr == lastQuery )
        return;
    lastQuery = queryStr;

    _matches.clear();
    if ( _emptyQuery )
        return;
    _matches = Exif::Database::instance()->filesMatchingQuery( queryStr );
}

bool Exif::SearchInfo::matches( const QString& fileName ) const
{
    if ( _emptyQuery )
        return true;

    return _matches.contains( fileName );
}

void Exif::SearchInfo::addCamara( const QValueList< QPair<QString,QString> >& list )
{
    _cameras = list;
}

QString Exif::SearchInfo::buildCameraSearchQuery() const
{
    QStringList subResults;
    for( QValueList< QPair<QString,QString> >::ConstIterator cameraIt = _cameras.begin(); cameraIt != _cameras.end(); ++cameraIt ) {
        subResults.append( QString::fromLatin1( "(Exif_Image_Make='%1' and Exif_Image_Model='%2')" )
                           .arg( (*cameraIt).first).arg( (*cameraIt).second ) );
    }
    if ( subResults.count() != 0 )
        return QString::fromLatin1( "(%1)" ).arg( subResults.join( QString::fromLatin1( " or " ) ) );
    else
        return QString::null;
}

