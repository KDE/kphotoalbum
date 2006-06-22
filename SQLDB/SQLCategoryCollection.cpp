#include "SQLCategoryCollection.h"
#include "SQLCategory.h"
#include "QueryUtil.h"
#include <qsqlquery.h>
#include "QueryHelper.h"
#include "SQLFolderCategory.h"

DB::CategoryPtr SQLDB::SQLCategoryCollection::categoryForName( const QString& name ) const
{
#ifndef HASKEXIDB
    return KSharedPtr<DB::Category>( new SQLCategory( idForCategory(name) ) );
#else
    if (name == "Folder") {
        return DB::CategoryPtr(new SQLFolderCategory());
    }
    else {
        return DB::CategoryPtr(new SQLCategory(QueryHelper::instance()->
                                               idForCategory(name)));
    }
#endif
}

QStringList SQLDB::SQLCategoryCollection::categoryNames() const
{
#ifndef HASKEXIDB
    return runAndReturnList( "SELECT distinct category FROM categorysetup" );
#else
    QStringList l = QueryHelper::instance()->
        executeQuery("SELECT name FROM category").asStringList();
    l.prepend("Folder");
    // TODO: Tokens
    return l;
#endif
}

void SQLDB::SQLCategoryCollection::removeCategory( const QString& name )
{
#ifndef HASKEXIDB
    QSqlQuery query;
    query.prepare( "DELETE FROM categorysetup WHERE category = :category" );
    query.bindValue( QString::fromLatin1( ":catgory" ), name );
    if ( !query.exec() )
        showError( query );
#else
    if (name == "Folder")
        return;
    QueryHelper::instance()->
        executeStatement("DELETE FROM category WHERE name=%s",
                         QueryHelper::Bindings() << name);
#endif
}

void SQLDB::SQLCategoryCollection::rename( const QString& oldName, const QString& newName )
{
#ifndef HASKEXIDB
    QSqlQuery query;
    query.prepare( "UPDATE categorysetup SET category = :newName WHERE category = :oldName" );
    query.bindValue( QString::fromLatin1( ":oldName" ), oldName );
    query.bindValue( QString::fromLatin1( ":newName" ), newName );
    if ( !query.exec() )
        showError( query );
#else
    if (oldName == "Folder")
        return;
    QueryHelper::instance()->
        executeStatement("UPDATE category SET name=%s WHERE name=%s",
                         QueryHelper::Bindings() << newName << oldName);
#endif
}

QValueList<DB::CategoryPtr> SQLDB::SQLCategoryCollection::categories() const
{
    QStringList cats = categoryNames();
    QValueList<DB::CategoryPtr> result;
    for( QStringList::ConstIterator it = cats.begin(); it != cats.end(); ++it ) {
        result.append( categoryForName( *it ) );
    }
    return result;
}

void SQLDB::SQLCategoryCollection::addCategory( const QString& category, const QString& icon, DB::Category::ViewSize size,
                                                DB::Category::ViewType type, bool showIt )
{
#ifndef HASKEXIDB
    QString queryStr = QString::fromLatin1( "SELECT MAX(categoryId) FROM categorysetup" );
    int idx = fetchItem( queryStr ).toInt() + 1;

    QSqlQuery query;
    query.prepare( QString::fromLatin1( "INSERT into categorysetup set category=:category, viewtype=:viewtype,"
                                        "viewsize=:viewsize, icon=:icon, showIt=:showIt" ) );
    query.bindValue( ":category", category );
    query.bindValue( ":icon", icon );
    query.bindValue( ":viewsize", size );
    query.bindValue( ":viewtype", type );
    query.bindValue( ":showIt", showIt );
    query.bindValue( ":categoryId", idx );
    if ( !query.exec() )
        showError( query );
#else
    if (category == "Folder")
        return;
    QueryHelper::instance()->
        executeStatement("INSERT INTO category(name, icon, "
                         "visible, viewtype, viewsize) "
                         "VALUES(%s, %s, %s, %s, %s)",
                         QueryHelper::Bindings() << category << icon <<
                         size << type << showIt);
#endif
}

#include "SQLCategoryCollection.moc"
