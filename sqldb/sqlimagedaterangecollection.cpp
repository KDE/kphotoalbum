#include "sqlimagedaterangecollection.h"
#include <qvariant.h>
#include <qmap.h>
#include "queryutil.h"
ImageCount SQLImageDateRangeCollection::count( const ImageDate& from, const ImageDate& to )
{
    QString queryStr = QString::fromLatin1( "SELECT count(*) from imageinfo WHERE startDate >= :startDate and endDate <= :endDate" );
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":startDate" ), from.min() );
    map.insert( QString::fromLatin1( ":endDate" ), to.max() );
    int exact = SQLDB::fetchItem( queryStr, map ).toInt();

    queryStr = QString::fromLatin1( "SELECT count(*) from imageinfo WHERE endDate >= :startDate and startDate <= :endDate" );
    int range = SQLDB::fetchItem( queryStr, map ).toInt() - exact;
    return ImageCount( exact, range );
}

QDateTime SQLImageDateRangeCollection::lowerLimit() const
{
    return QDateTime();
}

QDateTime SQLImageDateRangeCollection::upperLimit() const
{
    return QDateTime();
}
