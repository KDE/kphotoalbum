#include "SQLCategoryCollection.h"
#include "SQLCategory.h"
#include <qsqlquery.h>
#include "QueryHelper.h"
#include "SQLFolderCategory.h"
#include "SQLSpecialCategory.h"

DB::CategoryPtr SQLDB::SQLCategoryCollection::categoryForName( const QString& name ) const
{
    if (name == "Folder") {
        return DB::CategoryPtr(new SQLFolderCategory());
    }
    int categoryId = QueryHelper::instance()->idForCategory(name);
    if (name == "Tokens") {
        return DB::CategoryPtr(new SQLSpecialCategory(categoryId));
    }
    else {
        return DB::CategoryPtr(new SQLCategory(categoryId));
    }
}

QStringList SQLDB::SQLCategoryCollection::categoryNames() const
{
    QStringList l = QueryHelper::instance()->
        executeQuery("SELECT name FROM category").asStringList();
    l.prepend("Folder");
    // Tokens is in category table
    l.sort();
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

void SQLDB::SQLCategoryCollection::rename(const QString& oldName, const QString& newName)
{
    if (oldName == "Folder" || oldName == "Tokens" ||
        newName == "Folder" || newName == "Tokens")
        return;

    categoryForName(oldName)->setName(newName);
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
    // Tokens isn't special here.
    if (category == "Folder")
        return;
    QueryHelper::instance()->
        executeStatement("DELETE FROM category WHERE name=%s",
                         QueryHelper::Bindings() << category);
    QueryHelper::instance()->
        executeStatement("INSERT INTO category(name, icon, "
                         "visible, viewtype, viewsize) "
                         "VALUES(%s, %s, %s, %s, %s)",
                         QueryHelper::Bindings() << category << icon <<
                         size << type << showIt);
}

#include "SQLCategoryCollection.moc"
