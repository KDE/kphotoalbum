#include "Exif/SearchInfo.h"
#include <qsqlquery.h>
#include "SearchInfo.h"

void Exif::SearchInfo::addSearchKey( const QString& key, const QValueList<int> values )
{
    _intKeys.append( qMakePair( key, values ) );
}


QStringList Exif::SearchInfo::matches()
{
    QStringList result;

    QSqlQuery query( buildQuery() );
    if ( !query.exec() )
        qDebug("Failiure");

    while ( query.next() ) {
        result.append( query.value(0).toString() );
    }

    return result;
}


QStringList Exif::SearchInfo::buildIntKeyQuery()
{
    QStringList andArgs;
    for( QValueList< QPair<QString, QValueList<int> > >::Iterator intIt = _intKeys.begin(); intIt != _intKeys.end(); ++intIt ) {
        QStringList orArgs;
        QString key = (*intIt).first;
        QValueList<int> values =(*intIt).second;

        qDebug("length 2= %d", values.count() );
        for( QValueList<int>::Iterator argIt = values.begin(); argIt != values.end(); ++argIt ) {
            orArgs << QString::fromLatin1( "(%1 == %2)" ).arg( key ).arg( *argIt );
            qDebug( "%s", QString::fromLatin1( "(%1 == %2)" ).arg( key ).arg( *argIt ).latin1());
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

QString Exif::SearchInfo::buildQuery()
{
    QStringList subQueries;
    subQueries += buildIntKeyQuery();
    subQueries += buildRangeQuery();

    QString query = QString::fromLatin1( "SELECT filename from exif WHERE %1" )
                    .arg( subQueries.join( QString::fromLatin1( " and " ) ) );

    qDebug("%s", query.latin1() );
    return query;
}

QStringList Exif::SearchInfo::buildRangeQuery()
{
    QStringList result;
    for( RangeList::ConstIterator it = _rangeKeys.begin(); it != _rangeKeys.end(); ++it ) {
        QString str = sqlForOneRangeItem( *it );
        if ( !str.isNull() )
            result.append( str );
    }
    return result;
}


QString Exif::SearchInfo::sqlForOneRangeItem( const Range& range )
{
    // Notive I multiply factors on each value to ensure that we do not fail due to rounding errors for say 1/3

    if ( range.isLowerMin ) {
        //  Min to Min  means < x
        if ( range.isUpperMin )
            return QString::fromLatin1( "%1 < %2" ).arg( range.key ).arg( range.min *1.01 );

        //  Min to Max means all images
        if ( range.isUpperMax )
            return QString::null;

        //  Min to y   means <= y
        return QString::fromLatin1( "%1 <= %2" ).arg( range.key ).arg( range.max * 1.01 );
    }

    //  MAX to MAX   means y
    if ( range.isLowerMax )
        return QString::fromLatin1( "%1 > %2" ).arg( range.key ).arg( range.max*1.01 );

    //  x to Max   means >= x
    if ( range.isUpperMax )
        return QString::fromLatin1( "%1 >= %2" ).arg( range.key ).arg( range.min *0.99 );

    //  x to y     means >=x and <=y
    return QString::fromLatin1( "(%1 <= %2 and %3 <= %4)" )
        .arg( range.min * 0.99 )
        .arg( range.key ).arg( range.key )
        .arg( range.max * 1.01);
}

