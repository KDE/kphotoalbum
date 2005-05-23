#include "query.h"
#include <membermap.h>
#include <imagedb.h>
#include <imagesearchinfo.h>
#include <qsqlquery.h>
QStringList SQLDB::buildQueries( OptionMatcher* matcher )
{
    QStringList allImages;
    allImages << QString::fromLatin1( "SELECT distinct fileName FROM sortorder" );

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
    QString prefix = QString::fromLatin1( "SELECT distinct q0.fileName FROM " );
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
            filesMatch += QString::fromLatin1( "q0.fileName = q%1.fileName" ).arg( idx );
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
        result += QString::fromLatin1( " and q0.fileName not in ( SELECT fileName FROM imagecategoryinfo WHERE %1)" )
                  .arg( buildValue( (*it)->_category, list, -1, true ) );
    }
    return result;
}

QString SQLDB::buildValue( const QString& category, const QStringList& vals, int idx, bool negate )
{
    QString expression;
    QString prefix;
    if ( idx != -1 )
        prefix = QString::fromLatin1("q%1.").arg( idx );

    for( QStringList::ConstIterator it = vals.begin(); it != vals.end(); ++it ) {
        if ( !expression.isEmpty() )
            expression += QString::fromLatin1( " or " );
        expression += QString::fromLatin1( "%1value = \"%2\"" ).arg( prefix ).arg( *it );
    }
    if ( negate )
        expression = QString::fromLatin1( "!(%1)" ).arg( expression );
    else
        expression = QString::fromLatin1( "(%1)" ).arg( expression );

    return QString::fromLatin1( "%1category = \"%2\" and %3 " )
        .arg(prefix).arg( category ).arg( expression );
}

QStringList SQLDB::filesMatchingQuery( const ImageSearchInfo& info )
{
    QStringList result;
    QStringList queries = buildQueries( info.query() );
    for( QStringList::Iterator it = queries.begin(); it != queries.end(); ++it ) {
        QSqlQuery query;
        if ( !query.exec( *it ) )
            showError( query.lastError(), *it );

        while ( query.next() ) {
            QString str = query.value(0).toString();
            if ( !result.contains( str ) ) // PENDING(blackie) ouch O(n^2) complexity, do better!
                result.append( str );
        }
    }
    return result;
}

void SQLDB::showError( const QSqlError& error, const QString& query )
{
    qFatal( "Error running query: %s\nError was: %s,%s", query.latin1(), error.driverText().latin1(), error.databaseText().latin1() );
}

QStringList SQLDB::values( OptionValueMatcher* matcher )
{
    QStringList values;
    values.append( matcher->_option );

    if ( ImageDB::instance()->memberMap().isGroup( matcher->_category, matcher->_option ) )
        values += ImageDB::instance()->memberMap().members( matcher->_category, matcher->_option, true );
    return values;
}

