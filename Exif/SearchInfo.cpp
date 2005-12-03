#include "Exif/SearchInfo.h"
#include <qsqlquery.h>

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


QString Exif::SearchInfo::buildQuery()
{
    QStringList andArgs;
    qDebug("length 1= %d", _intKeys.count() );
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

    QString query = QString::fromLatin1( "SELECT filename from exif WHERE %1" )
                    .arg( andArgs.join( QString::fromLatin1( " and " ) ) );

    qDebug("%s", query.latin1() );
    return query;
}
