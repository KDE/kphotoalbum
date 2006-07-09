#include "SQLCategoryCollection.h"
#include "SQLCategory.h"
#include <qsqlquery.h>
#include "QueryHelper.h"
#include "SQLFolderCategory.h"

DB::CategoryPtr SQLDB::SQLCategoryCollection::categoryForName( const QString& name ) const
{
    if (name == "Folder") {
        return DB::CategoryPtr(new SQLFolderCategory());
    }
    else {
        return DB::CategoryPtr(new SQLCategory(QueryHelper::instance()->
                                               idForCategory(name)));
    }
}

QStringList SQLDB::SQLCategoryCollection::categoryNames() const
{
    QStringList l = QueryHelper::instance()->
        executeQuery("SELECT name FROM category").asStringList();
    l.prepend("Folder");
    // TODO: Tokens
    return l;
}

void SQLDB::SQLCategoryCollection::removeCategory( const QString& name )
{
    if (name == "Folder")
        return;
    QueryHelper::instance()->
        executeStatement("DELETE FROM category WHERE name=%s",
                         QueryHelper::Bindings() << name);
}

void SQLDB::SQLCategoryCollection::rename( const QString& oldName, const QString& newName )
{
    if (oldName == "Folder")
        return;
    QueryHelper::instance()->
        executeStatement("UPDATE category SET name=%s WHERE name=%s",
                         QueryHelper::Bindings() << newName << oldName);
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
    if (category == "Folder")
        return;
    QueryHelper::instance()->
        executeStatement("INSERT INTO category(name, icon, "
                         "visible, viewtype, viewsize) "
                         "VALUES(%s, %s, %s, %s, %s)",
                         QueryHelper::Bindings() << category << icon <<
                         size << type << showIt);
}

#include "SQLCategoryCollection.moc"
