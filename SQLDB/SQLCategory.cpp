#include "SQLCategory.h"
#include <kdebug.h>
#include <qsqlquery.h>
#include "QueryUtil.h"

QString SQLDB::SQLCategory::name() const
{
    return categoryForId(_categoryId);
}

void SQLDB::SQLCategory::setName( const QString& /*name*/ )
{
    // PENDING(blackie) do I need to update the DB?
    kdDebug() << "What should I do here?!\n";
}

QString SQLDB::SQLCategory::iconName() const
{
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":categoryId" ), _categoryId  );
    return fetchItem( QString::fromLatin1( "SELECT icon FROM categorysetup WHERE categoryId = :categoryId" ), map ).toString();
}

void SQLDB::SQLCategory::setIconName( const QString& name )
{
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":icon" ), name );
    map.insert( QString::fromLatin1( ":categoryId" ), _categoryId );
    runQuery( "UPDATE categorysetup set icon = :icon WHERE categoryId = :categoryId", map );
}

DB::Category::ViewSize SQLDB::SQLCategory::viewSize() const
{
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":categoryId" ), _categoryId  );
    return (DB::Category::ViewSize) fetchItem( QString::fromLatin1( "SELECT viewsize FROM categorysetup WHERE categoryId = :categoryId" ), map ).toInt();
}

void SQLDB::SQLCategory::setViewSize( ViewSize size )
{
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":size" ), size );
    map.insert( QString::fromLatin1( ":categoryId" ), _categoryId );
    runQuery( "UPDATE categorysetup set viewsize = :size WHERE categoryId = :categoryId", map );
}

void SQLDB::SQLCategory::setViewType( ViewType type )
{
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":type" ), type );
    map.insert( QString::fromLatin1( ":categoryId" ), _categoryId );
    runQuery( "UPDATE categorysetup set viewtype = :type WHERE categoryId = :categoryId", map );
}

DB::Category::ViewType SQLDB::SQLCategory::viewType() const
{
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":categoryId" ), _categoryId  );
    return (DB::Category::ViewType) fetchItem( QString::fromLatin1( "SELECT viewtype FROM categorysetup WHERE categoryId = :categoryId" ), map ).toInt();
}

void SQLDB::SQLCategory::setDoShow( bool b )
{
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":showit" ), b );
    map.insert( QString::fromLatin1( ":categoryId" ), _categoryId );
    runQuery( "UPDATE categorysetup set showit = :showit WHERE categoryId = :categoryId", map );
}

bool SQLDB::SQLCategory::doShow() const
{
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":categoryId" ), _categoryId  );
    return  fetchItem( QString::fromLatin1( "SELECT showit FROM categorysetup WHERE categoryId = :categoryId" ), map ).toBool();

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
    QString queryStr = QString::fromLatin1( "DELETE FROM categorysortorder WHERE categoryId = :categoryId" );
    QMap<QString, QVariant> map;
    map.insert( QString::fromLatin1( ":categoryId" ), _categoryId );
    runQuery( queryStr, map );

    QSqlQuery query;
    query.prepare( QString::fromLatin1( "INSERT INTO categorysortorder set idx = :idx, categoryId = :categoryId, item = :item" ) );
    query.bindValue( QString::fromLatin1( ":categoryId" ), _categoryId );
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
    query.prepare( QString::fromLatin1( "INSERT INTO categorysortorder set idx = :idx, categoryId = :categoryId, item = :item" ) );
    query.bindValue( QString::fromLatin1( ":categoryId" ), _categoryId );
    query.bindValue( QString::fromLatin1( ":idx" ), idx++ );
    query.bindValue( QString::fromLatin1( ":item" ), item );
    if ( !query.exec() )
        showError( query );
}

QStringList SQLDB::SQLCategory::items() const
{
    QString query = QString::fromLatin1( "SELECT item FROM categorysortorder WHERE categoryId = :categoryId" ); // SORT BY idx" );
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":categoryId" ), _categoryId );
    return runAndReturnList( query, map );
}

SQLDB::SQLCategory::SQLCategory( int categoryId )
    : _categoryId( categoryId )
{
}
