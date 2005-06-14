#include "query.h"
#include <membermap.h>
#include <imagedb.h>
#include <imagesearchinfo.h>
#include <qsqlquery.h>
#include <options.h>
QStringList SQLDB::buildQueries( OptionMatcher* matcher )
{
    QStringList allImages;
    allImages << QString::fromLatin1( "SELECT distinct fileId FROM sortorder" );

    if ( matcher == 0 )
        return allImages;

    OptionAndMatcher* andMatcher;
    if ( ( andMatcher = dynamic_cast<OptionAndMatcher*>( matcher ) ) )
        return QStringList() << buildAndQuery( andMatcher );

    OptionOrMatcher* orMatcher = dynamic_cast<OptionOrMatcher*>( matcher );
    if ( !orMatcher ) {
        OptionAndMatcher andMatcher;
        andMatcher.addElement( matcher );
        QString result = buildAndQuery( &andMatcher );
        andMatcher._elements.clear();
        return QStringList() << result;
    }

    QStringList result;
    for( QValueList<OptionMatcher*>::ConstIterator it = orMatcher->_elements.begin(); it != orMatcher->_elements.end(); ++it ) {
        OptionAndMatcher* subMatcher = dynamic_cast<OptionAndMatcher*>( *it );
        if ( !subMatcher )
            qWarning("Internal Error: the child matcher for an 'or' query was not an 'and' query" );
        else
            result += buildAndQuery( subMatcher );
    }
    return result;
}

QString SQLDB::buildAndQuery( OptionAndMatcher* matcher )
{
    QString prefix = QString::fromLatin1( "SELECT distinct q0.fileId FROM " );
    QString query;
    QString filesMatch;
    int idx = 0;
    QMap<QString,QStringList> matchedValues;
    QValueList<OptionEmptyMatcher*> emptyMatchers;

    for( QValueList<OptionMatcher*>::ConstIterator it = matcher->_elements.begin(); it != matcher->_elements.end(); ++it, ++idx ) {
        if ( idx != 0 ) {
            prefix += QString::fromLatin1( ", " );
            if ( idx > 1 )
                filesMatch += QString::fromLatin1( "and " );
            filesMatch += QString::fromLatin1( "q0.fileId = q%1.fileId" ).arg( idx );
        }
        prefix += QString::fromLatin1( "imagecategoryinfo q%1" ).arg( idx );

        OptionValueMatcher* valueMatcher;
        OptionNotMatcher* notMatcher;
        OptionEmptyMatcher* emptyMatcher;

        if ( (valueMatcher = dynamic_cast<OptionValueMatcher*>( *it ) ) ) {
            if ( idx != 0 )
                query += QString::fromLatin1( "and " );

            query += buildValue( valueMatcher->_category, values( valueMatcher), idx, false );
            matchedValues[valueMatcher->_category] += values( valueMatcher );
        }
        else if ( ( notMatcher = dynamic_cast<OptionNotMatcher*>( *it ) ) ) {
            if ( idx != 0 )
                query += QString::fromLatin1( "and " );

            OptionMatcher* child = notMatcher->_element;
            OptionValueMatcher* childValue = dynamic_cast<OptionValueMatcher*>( child );
            if ( !childValue ) {
                qWarning( "Internal Error: child of not matcher was not a value matcher" );
                child->debug(0);
            }
            else
                query += buildValue( childValue->_category, values( childValue ), idx, true );
        }
        else if ( ( emptyMatcher = dynamic_cast<OptionEmptyMatcher*>( *it ) ) ) {
            emptyMatchers.append( emptyMatcher );
        }

        else {
            qWarning("Internal Error: Unknown node for buildAndQuery" );
            (*it)->debug(0);
        }
    }

    QString possibleAnd;
    if ( matcher->_elements.count() > 1 )
        possibleAnd = QString::fromLatin1( "and" );

    QString result = QString::fromLatin1( "%1 WHERE %2 %3 %4").arg( prefix ).arg( filesMatch ).arg( possibleAnd ).arg ( query );

    for( QValueList<OptionEmptyMatcher*>::Iterator it = emptyMatchers.begin(); it != emptyMatchers.end(); ++it ) {
        QStringList list = matchedValues[(*it)->_category ];
        result += QString::fromLatin1( " %1 q0.fileId not in ( SELECT fileId FROM imagecategoryinfo WHERE %2)" )
                  .arg( query.isEmpty() ? QString::null : QString::fromLatin1( "and" ) ).arg( buildValue( (*it)->_category, list, -1, true ) );
    }
    return result;
}

QString SQLDB::buildValue( const QString& category, const QStringList& vals, int idx, bool negate )
{
    QString expression;
    QString prefix;
    if ( idx != -1 )
        prefix = QString::fromLatin1("q%1.").arg( idx );

    if ( !vals.isEmpty() ) {
        for( QStringList::ConstIterator it = vals.begin(); it != vals.end(); ++it ) {
            if ( !expression.isEmpty() )
                expression += QString::fromLatin1( " or " );
            expression += QString::fromLatin1( "%1value = \"%2\"" ).arg( prefix ).arg( *it );
        }
        if ( negate )
            expression = QString::fromLatin1( "!(%1)" ).arg( expression );
        else
            expression = QString::fromLatin1( "(%1)" ).arg( expression );

        return QString::fromLatin1( "%1categoryId = \"%2\" and %3 " )
            .arg(prefix).arg( idForCategory(category) ).arg( expression );
    }
    else
        return QString::fromLatin1( "%1categoryId = \"%2\" " ).arg(prefix).arg( idForCategory(category) );
}

QValueList<int> SQLDB::filesMatchingQuery( const ImageSearchInfo& info )
{
    QValueList<int> result;
    QStringList queries = buildQueries( info.query() ); // PENDING(blackie) How about bindvalue here?
    for( QStringList::Iterator it = queries.begin(); it != queries.end(); ++it ) {
        QSqlQuery query;
        if ( !query.exec( *it ) )
            showError( query );

        while ( query.next() ) {
            int val = query.value(0).toInt();
            if ( !result.contains( val ) ) // PENDING(blackie) ouch O(n^2) complexity, do better!
                result.append( val );
        }
    }
    return result;
}

void SQLDB::showError( QSqlQuery& query )
{
    qFatal( "Error running query: %s\nError was: %s", query.executedQuery().latin1(), query.lastError().text().latin1());
}

QStringList SQLDB::values( OptionValueMatcher* matcher )
{
    QStringList values;
    values.append( matcher->_option );

    if ( ImageDB::instance()->memberMap().isGroup( matcher->_category, matcher->_option ) )
        values += ImageDB::instance()->memberMap().members( matcher->_category, matcher->_option, true );
    return values;
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
    QStringList result;
    while ( query.next() )
        result.append( query.value(0).toString() );
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

QStringList SQLDB::memberOfCategory( const QString& category )
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

