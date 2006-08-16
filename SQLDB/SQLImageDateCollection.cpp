#include "SQLImageDateCollection.h"
#include <qvariant.h>
#include <qmap.h>
#include "QueryHelper.h"

using namespace SQLDB;

SQLImageDateCollection::SQLImageDateCollection(Connection& connection):
    _qh(connection)
{
}

DB::ImageCount SQLImageDateCollection::count( const DB::ImageDate& range )
{
    // In a perfect world, we should check that the db hasn't changed, but
    // as we will get a new instance of this class each time the search
    // changes, it is really not that important, esp. because it is only
    // for the datebar, where a bit out of sync doens't matter too much.

    static QMap<DB::ImageDate, DB::ImageCount> cache;
    if ( cache.contains( range ) )
        return cache[range];

    int exact =
        _qh.executeQuery("SELECT count(*) FROM media "
                         "WHERE %s<=startTime AND endTime<=%s",
                         QueryHelper::Bindings() <<
                         range.start() << range.end()
                         ).firstItem().toInt();
    int rng =
        _qh.executeQuery("SELECT count(*) FROM media "
                         "WHERE %s<=endTime AND startTime<=%s",
                         QueryHelper::Bindings() <<
                         range.start() << range.end()
                         ).firstItem().toInt() - exact;
    DB::ImageCount result( exact, rng );
    cache.insert( range, result );
    return result;
}

QDateTime SQLImageDateCollection::lowerLimit() const
{
    static QDateTime cachedLower;
    if (cachedLower.isNull())
        cachedLower = _qh.executeQuery("SELECT min(startTime) FROM media"
                                       ).firstItem().toDateTime();
    return cachedLower;
}

QDateTime SQLImageDateCollection::upperLimit() const
{
    static QDateTime cachedUpper;
    if (cachedUpper.isNull())
        cachedUpper = _qh.executeQuery("SELECT max(endTime) FROM media"
                                       ).firstItem().toDateTime();
    return cachedUpper;
}
