#include "SQLImageDateCollection.h"
#include <qvariant.h>
#include <qmap.h>
#include "QueryUtil.h"
#include "QueryHelper.h"
#include "config.h" // HASKEXIDB

using SQLDB::QueryHelper;

DB::ImageCount SQLImageDateCollection::count( const DB::ImageDate& range )
{
    // In a perfect world, we should check that the db hasn't changed, but
    // as we will get a new instance of this class each time the search
    // changes, it is really not that important, esp. because it is only
    // for the datebar, where a bit out of sync doens't matter too much.

    static QMap<DB::ImageDate, DB::ImageCount> cache;
    if ( cache.contains( range ) )
        return cache[range];

#ifndef HASKEXIDB
    QString queryStr = QString::fromLatin1( "SELECT count(*) from imageinfo WHERE startDate >= :startDate and endDate <= :endDate" );
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":startDate" ), range.start() );
    map.insert( QString::fromLatin1( ":endDate" ), range.end() );
    int exact = SQLDB::fetchItem( queryStr, map ).toInt();

    queryStr = QString::fromLatin1( "SELECT count(*) from imageinfo WHERE endDate >= :startDate and startDate <= :endDate" );
    int rng = SQLDB::fetchItem( queryStr, map ).toInt() - exact;
#else
    int exact = QueryHelper::instance()->
        executeQuery("SELECT count(*) FROM media "
                     "WHERE %s<=startTime AND endTime<=%s",
                     QueryHelper::Bindings() << range.start() << range.end()).
        firstItem().toInt();
    int rng = QueryHelper::instance()->
        executeQuery("SELECT count(*) FROM media "
                     "WHERE %s<=endTime AND startTime<=%s",
                     QueryHelper::Bindings() << range.start() << range.end()).
        firstItem().toInt() - exact;
#endif
    DB::ImageCount result( exact, rng );
    cache.insert( range, result );
    return result;
}

QDateTime SQLImageDateCollection::lowerLimit() const
{
    static QDateTime cachedLower;
    if (cachedLower.isNull())
        cachedLower = QueryHelper::instance()->
            executeQuery("SELECT min(startTime) FROM media").
            firstItem().toDateTime();
    return cachedLower;
}

QDateTime SQLImageDateCollection::upperLimit() const
{
    static QDateTime cachedUpper;
    if (cachedUpper.isNull())
        cachedUpper = QueryHelper::instance()->
            executeQuery("SELECT max(endTime) FROM media").
            firstItem().toDateTime();
    return cachedUpper;
}
