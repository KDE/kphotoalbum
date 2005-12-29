#include "QueryUtil.h"
#include <membermap.h>
#include <imagedb.h>
#include <imagesearchinfo.h>
#include <qsqlquery.h>
#include <options.h>

void SQLDB::showError( QSqlQuery& query )
{
    qFatal( "Error running query: %s\nError was: %s", query.executedQuery().latin1(), query.lastError().text().latin1());
}

QStringList SQLDB::runAndReturnList( const QString& queryString, const QMap<QString,QVariant>& bindings )
{
    QSqlQuery query;
    query.prepare( queryString );
    for( QMap<QString,QVariant>::ConstIterator it = bindings.begin(); it != bindings.end(); ++it ) {
        query.bindValue( it.key(), it.data() );
    }
    if ( !query.exec() ) {
        showError( query );
        return QStringList();
    }
    // qDebug("%s", query.executedQuery().latin1());

    QStringList result;
    while ( query.next() )
        result.append( query.value(0).toString() );
    return result;
}

QValueList<int> SQLDB::runAndReturnIntList( const QString& queryString, const QMap<QString,QVariant>& bindings )
{
    QValueList<int> result;

    QSqlQuery query;
    query.prepare( queryString );
    for( QMap<QString,QVariant>::ConstIterator it = bindings.begin(); it != bindings.end(); ++it ) {
        query.bindValue( it.key(), it.data() );
    }
    if ( !query.exec() ) {
        showError( query );
        return result;
    }
    while ( query.next() )
        result.append( query.value(0).toInt() );
    return result;
}

QVariant SQLDB::fetchItem( const QString& queryString, const QMap<QString,QVariant>& bindings )
{
    QSqlQuery query;
    if ( !runQuery( queryString, bindings, query ) )
        return QVariant();

    if ( query.next() )
        return query.value(0);
    return QVariant();
}

bool SQLDB::runQuery( const QString& queryString, const QMap<QString,QVariant>& bindings, QSqlQuery& query )
{
    query.prepare( queryString );
    for( QMap<QString,QVariant>::ConstIterator it = bindings.begin(); it != bindings.end(); ++it ) {
        query.bindValue( it.key(), it.data() );
    }
    if ( !query.exec() ) {
        showError( query );
        return false;
    }
    return true;
}

bool SQLDB::runQuery( const QString& queryString, const QMap<QString,QVariant>& bindings )
{
    QSqlQuery query;
    return runQuery( queryString, bindings, query );
}

QStringList SQLDB::membersOfCategory( const QString& category )
{
    QString query = QString::fromLatin1( "SELECT item FROM categorysortorder WHERE category=:category" );
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":category" ), category );
    return runAndReturnList( query, map );
}

QString SQLDB::fileNameForId( int id, bool fullPath )
{
    QString query = QString::fromLatin1( "SELECT fileName FROM sortorder WHERE fileId = :id" );
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":id" ), id );
    QString fileName = fetchItem( query, map ).toString();
    if ( fullPath )
        return Options::instance()->imageDirectory() + fileName;
    else
        return fileName;
}

int SQLDB::idForFileName( const QString& relativePath )
{
    QString query = QString::fromLatin1( "SELECT fileId from sortorder WHERE fileName = :fileName " );
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":fileName" ), relativePath );
    return fetchItem( query, map ).toInt();
}

QString SQLDB::categoryForId( int id )
{
    QString query = QString::fromLatin1( "SELECT category FROM categorysetup WHERE categoryId = :categoryId" );
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":categoryId" ), id );
    return fetchItem( query, map ).toString();
}

int SQLDB::idForCategory( const QString& category )
{
    QString query = QString::fromLatin1( "SELECT categoryId from categorysetup WHERE category = :category " );
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":category" ), category );
    return fetchItem( query, map ).toInt();
}

QValueList<int> SQLDB::allImages()
{
    return runAndReturnIntList( QString::fromLatin1( "SELECT * from sortorder" ) );
}

