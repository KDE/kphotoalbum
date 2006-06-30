#include "SQLCategory.h"
#include <kdebug.h>
#include <qsqlquery.h>
#include "QueryUtil.h"
#include "QueryHelper.h"

QString SQLDB::SQLCategory::name() const
{
#ifndef HASKEXIDB
    return categoryForId(_categoryId);
#else
    return QueryHelper::instance()->categoryForId(_categoryId);
#endif
}

void SQLDB::SQLCategory::setName( const QString& /*name*/ )
{
    // TODO: this
    // PENDING(blackie) do I need to update the DB?
    kdDebug() << "What should I do here?!\n";
}

QString SQLDB::SQLCategory::iconName() const
{
#ifndef HASKEXIDB
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":categoryId" ), _categoryId  );
    return fetchItem( QString::fromLatin1( "SELECT icon FROM categorysetup WHERE categoryId = :categoryId" ), map ).toString();
#else
    return QueryHelper::instance()->
        executeQuery("SELECT icon FROM category WHERE id=%s",
                     QueryHelper::Bindings() << _categoryId).
        firstItem().toString();
#endif
}

void SQLDB::SQLCategory::setIconName( const QString& name )
{
#ifndef HASKEXIDB
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":icon" ), name );
    map.insert( QString::fromLatin1( ":categoryId" ), _categoryId );
    runQuery( "UPDATE categorysetup set icon = :icon WHERE categoryId = :categoryId", map );
#else
    QueryHelper::instance()->
        executeStatement("UPDATE category SET icon=%s WHERE id=%s",
                         QueryHelper::Bindings() << name << _categoryId);
#endif
}

DB::Category::ViewSize SQLDB::SQLCategory::viewSize() const
{
#ifndef HASKEXIDB
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":categoryId" ), _categoryId  );
    return (DB::Category::ViewSize) fetchItem( QString::fromLatin1( "SELECT viewsize FROM categorysetup WHERE categoryId = :categoryId" ), map ).toInt();
#else
    return static_cast<DB::Category::ViewSize>
        (QueryHelper::instance()->
         executeQuery("SELECT viewsize FROM category WHERE id=%s",
                      QueryHelper::Bindings() << _categoryId).
         firstItem().toInt());
#endif
}

void SQLDB::SQLCategory::setViewSize( ViewSize size )
{
#ifndef HASKEXIDB
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":size" ), size );
    map.insert( QString::fromLatin1( ":categoryId" ), _categoryId );
    runQuery( "UPDATE categorysetup set viewsize = :size WHERE categoryId = :categoryId", map );
#else
    QueryHelper::instance()->
        executeStatement("UPDATE category SET viewsize=%s WHERE id=%s",
                         QueryHelper::Bindings() << size << _categoryId);
#endif
}

void SQLDB::SQLCategory::setViewType( ViewType type )
{
#ifndef HASKEXIDB
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":type" ), type );
    map.insert( QString::fromLatin1( ":categoryId" ), _categoryId );
    runQuery( "UPDATE categorysetup set viewtype = :type WHERE categoryId = :categoryId", map );
#else
    QueryHelper::instance()->
        executeStatement("UPDATE category SET viewtype=%s WHERE id=%s",
                         QueryHelper::Bindings() << type << _categoryId);
#endif
}

DB::Category::ViewType SQLDB::SQLCategory::viewType() const
{
#ifndef HASKEXIDB
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":categoryId" ), _categoryId  );
    return (DB::Category::ViewType) fetchItem( QString::fromLatin1( "SELECT viewtype FROM categorysetup WHERE categoryId = :categoryId" ), map ).toInt();
#else
    return static_cast<DB::Category::ViewType>
        (QueryHelper::instance()->
         executeQuery("SELECT viewtype FROM category WHERE id=%s",
                      QueryHelper::Bindings() << _categoryId).
         firstItem().toInt());
#endif
}

void SQLDB::SQLCategory::setDoShow( bool b )
{
#ifndef HASKEXIDB
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":showit" ), b );
    map.insert( QString::fromLatin1( ":categoryId" ), _categoryId );
    runQuery( "UPDATE categorysetup set showit = :showit WHERE categoryId = :categoryId", map );
#else
    QueryHelper::instance()->
        executeStatement("UPDATE category SET visible=%s WHERE id=%s",
                         QueryHelper::Bindings() << b << _categoryId);
#endif
}

bool SQLDB::SQLCategory::doShow() const
{
#ifndef HASKEXIDB
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":categoryId" ), _categoryId  );
    return  fetchItem( QString::fromLatin1( "SELECT showit FROM categorysetup WHERE categoryId = :categoryId" ), map ).toBool();
#else
    return QueryHelper::instance()->
        executeQuery("SELECT visible FROM category WHERE id=%s",
                     QueryHelper::Bindings() << _categoryId).
        firstItem().toInt();
#endif
}

void SQLDB::SQLCategory::setSpecialCategory( bool /*b*/ )
{
    // Can't be special category, so do nothing
}

bool SQLDB::SQLCategory::isSpecialCategory() const
{
    // Special categories have their own classes, so return false here
    return false;
}

void SQLDB::SQLCategory::setItems( const QStringList& items )
{
#ifndef HASKEXIDB
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
#else
    QueryHelper::instance()->
        executeStatement("DELETE FROM tag WHERE categoryId=%s",
                         QueryHelper::Bindings() << _categoryId);

    // TODO: set place too
    for(QStringList::const_iterator it = items.begin();
        it != items.end(); ++it ) {
        QueryHelper::instance()->
            executeStatement("INSERT INTO tag(name, categoryId) "
                             "VALUES(%s, %s)",
                             QueryHelper::Bindings() << *it << _categoryId);
    }
#endif
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
#ifndef HASKEXIDB
    QString queryStr = QString::fromLatin1( "SELECT MAX(idx) FROM categorysortorder" );
    int idx = fetchItem( queryStr ).toInt();

    QSqlQuery query;
    query.prepare( QString::fromLatin1( "INSERT INTO categorysortorder set idx = :idx, categoryId = :categoryId, item = :item" ) );
    query.bindValue( QString::fromLatin1( ":categoryId" ), _categoryId );
    query.bindValue( QString::fromLatin1( ":idx" ), idx++ );
    query.bindValue( QString::fromLatin1( ":item" ), item );
    if ( !query.exec() )
        showError( query );
#else
    QueryHelper::instance()->insertTag(_categoryId, item);
#endif
}

QStringList SQLDB::SQLCategory::items() const
{
#ifndef HASKEXIDB
    QString query = QString::fromLatin1( "SELECT item FROM categorysortorder WHERE categoryId = :categoryId" ); // SORT BY idx" );
    QMap<QString,QVariant> map;
    map.insert( QString::fromLatin1( ":categoryId" ), _categoryId );
    return runAndReturnList( query, map );
#else
    return QueryHelper::instance()->membersOfCategory(_categoryId);
#endif
}

SQLDB::SQLCategory::SQLCategory( int categoryId )
    : _categoryId( categoryId )
{
}
