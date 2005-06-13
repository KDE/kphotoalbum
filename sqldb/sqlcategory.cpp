#include "sqlcategory.h"
#include <kdebug.h>
#include <qsqlquery.h>
#include "query.h"
QString SQLDB::SQLCategory::name() const
{
    return _category;
}

void SQLDB::SQLCategory::setName( const QString& name )
{
    // PENDING(blackie) do I need to update the DB?
    kdDebug() << "Should I update the db here?";
    _category = name;
}

QString SQLDB::SQLCategory::iconName() const
{
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":category" ), _category  );
    return fetchItem( QString::fromLatin1( "SELECT icon FROM categorysetup WHERE category = :category" ), map ).toString();
}

void SQLDB::SQLCategory::setIconName( const QString& name )
{
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":icon" ), name );
    map.insert( QString::fromLatin1( ":category" ), _category );
    runQuery( "UPDATE categorysetup set icon = :icon WHERE category = :category", map );
}

Category::ViewSize SQLDB::SQLCategory::viewSize() const
{
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":category" ), _category  );
    return (Category::ViewSize) fetchItem( QString::fromLatin1( "SELECT viewsize FROM categorysetup WHERE category = :category" ), map ).toInt();
}

void SQLDB::SQLCategory::setViewSize( ViewSize size )
{
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":size" ), size );
    map.insert( QString::fromLatin1( ":category" ), _category );
    runQuery( "UPDATE categorysetup set viewsize = :size WHERE category = :category", map );
}

void SQLDB::SQLCategory::setViewType( ViewType type )
{
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":type" ), type );
    map.insert( QString::fromLatin1( ":category" ), _category );
    runQuery( "UPDATE categorysetup set viewtype = :type WHERE category = :category", map );
}

Category::ViewType SQLDB::SQLCategory::viewType() const
{
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":category" ), _category  );
    return (Category::ViewType) fetchItem( QString::fromLatin1( "SELECT viewtype FROM categorysetup WHERE category = :category" ), map ).toInt();
}

void SQLDB::SQLCategory::setDoShow( bool b )
{
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":showit" ), b );
    map.insert( QString::fromLatin1( ":category" ), _category );
    runQuery( "UPDATE categorysetup set showit = :showit WHERE category = :category", map );
}

bool SQLDB::SQLCategory::doShow() const
{
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":category" ), _category  );
    return  fetchItem( QString::fromLatin1( "SELECT showit FROM categorysetup WHERE category = :category" ), map ).toBool();

}

void SQLDB::SQLCategory::setSpecialCategory( bool /*b*/ )
{
    qDebug("NYI: void SQLDB::SQLCategory::setSpecialCategory( bool b )" );
}

bool SQLDB::SQLCategory::isSpecialCategory() const
{
    qDebug("NYI: bool SQLDB::SQLCategory::isSpecialCategory() const" );
    return false;
}

void SQLDB::SQLCategory::setItems( const QStringList& items )
{
    QString queryStr = QString::fromLatin1( "DELETE FROM categorysortorder WHERE category = :category" );
    QMap<QString, QVariant> map;
    map.insert( QString::fromLatin1( ":category" ), _category );
    runQuery( queryStr, map );

    QSqlQuery query;
    query.prepare( QString::fromLatin1( "INSERT INTO categorysortorder set idx = :idx, category = :category, item = :item" ) );
    query.bindValue( QString::fromLatin1( ":category" ), _category );
    int idx = 0;
    for( QStringList::ConstIterator it = items.begin(); it != items.end(); ++it ) {
        query.bindValue( QString::fromLatin1( ":idx" ), idx++ );
        query.bindValue( QString::fromLatin1( ":item" ), *it );
        if ( !query.exec() )
            showError( query );
    }
}

void SQLDB::SQLCategory::removeItem( const QString& /*item*/ )
{
    qDebug("NYI: void SQLDB::SQLCategory::removeItem( const QString& item )" );
}

void SQLDB::SQLCategory::renameItem( const QString& /*oldValue*/, const QString& /*newValue*/ )
{
    qDebug("NYI: void SQLDB::SQLCategory::renameItem( const QString& oldValue, const QString& newValue )" );
}

void SQLDB::SQLCategory::addItem( const QString& item )
{
    QString queryStr = QString::fromLatin1( "SELECT MAX(idx) FROM categorysortorder" );
    int idx = fetchItem( queryStr ).toInt();

    QSqlQuery query;
    query.prepare( QString::fromLatin1( "INSERT INTO categorysortorder set idx = :idx, category = :category, item = :item" ) );
    query.bindValue( QString::fromLatin1( ":category" ), _category );
    query.bindValue( QString::fromLatin1( ":idx" ), idx++ );
    query.bindValue( QString::fromLatin1( ":item" ), item );
    if ( !query.exec() )
        showError( query );
}

QStringList SQLDB::SQLCategory::items() const
{
    QString query = QString::fromLatin1( "SELECT item FROM categorysortorder WHERE category = :category" ); // SORT BY idx" );
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":category" ), _category );
    return runAndReturnList( query, map );
}

SQLDB::SQLCategory::SQLCategory( const QString& category )
    : _category( category )
{
}
