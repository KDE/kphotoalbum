#include "SQLImageDateRangeCollection.h"
#include <qvariant.h>
#include <qmap.h>
#include "QueryUtil.h"
ImageCount SQLImageDateRangeCollection::count( const ImageDateRange& range )
{
    // In a perfect world, we should check that the db hasn't changed, but
    // as we will get a new instance of this class each time the search
    // changes, it is really not that important, esp. because it is only
    // for the datebar, where a bit out of sync doens't matter too much.

    static QMap<ImageDateRange, ImageCount> cache;
    if ( cache.contains( range ) )
        return cache[range];

    QString queryStr = QString::fromLatin1( "SELECT count(*) from imageinfo WHERE startDate >= :startDate and endDate <= :endDate" );
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":startDate" ), range.date().start() );
    map.insert( QString::fromLatin1( ":endDate" ), range.date().end() );
    int exact = SQLDB::fetchItem( queryStr, map ).toInt();

    queryStr = QString::fromLatin1( "SELECT count(*) from imageinfo WHERE endDate >= :startDate and startDate <= :endDate" );
    int rng = SQLDB::fetchItem( queryStr, map ).toInt() - exact;
    ImageCount result( exact, rng );
    cache.insert( range, result );
    return result;
}

QDateTime SQLImageDateRangeCollection::lowerLimit() const
{
    return QDateTime();
}

QDateTime SQLImageDateRangeCollection::upperLimit() const
{
    return QDateTime();
}
