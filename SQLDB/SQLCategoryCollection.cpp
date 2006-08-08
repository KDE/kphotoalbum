#include "SQLCategoryCollection.h"
#include "SQLCategory.h"
#include "QueryHelper.h"
#include "SQLFolderCategory.h"
#include "SQLSpecialCategory.h"

DB::CategoryPtr SQLDB::SQLCategoryCollection::categoryForName( const QString& name ) const
{
    int categoryId;
    try {
        categoryId = QueryHelper::instance()->idForCategory(name);
    }
    catch (NotFoundError&) {
        return 0;
    }

    DB::CategoryPtr p;
    if (name == "Folder") {
        p = new SQLFolderCategory(categoryId);
    }
    else if (name == "Tokens") {
        p = new SQLSpecialCategory(categoryId);
    }
    else {
        p = new SQLCategory(categoryId);
    }

    connect(p, SIGNAL(changed()), this, SIGNAL(categoryCollectionChanged()));
    connect(p, SIGNAL(itemRemoved(const QString&)),
            this, SLOT(itemRemoved(const QString&)));
    connect(p, SIGNAL(itemRenamed(const QString&, const QString&)),
            this, SLOT(itemRenamed(const QString&, const QString&)));

    return p;
}

QStringList SQLDB::SQLCategoryCollection::categoryNames() const
{
    QStringList l = QueryHelper::instance()->
        executeQuery("SELECT name FROM category").asStringList();
    l.sort();
    return l;
}

void SQLDB::SQLCategoryCollection::removeCategory( const QString& name )
{
    int id;
    try {
        id = QueryHelper::instance()->idForCategory(name);
    }
    catch (NotFoundError&) {
        return;
    }
    QueryHelper::instance()->
        executeStatement("DELETE FROM tag WHERE categoryId=%s",
                         QueryHelper::Bindings() << id);
    QueryHelper::instance()->
        executeStatement("DELETE FROM category WHERE id=%s",
                         QueryHelper::Bindings() << id);

    emit categoryCollectionChanged();
}

void SQLDB::SQLCategoryCollection::rename(const QString& oldName, const QString& newName)
{
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
    QueryHelper::instance()->
        executeStatement("DELETE FROM category WHERE name=%s",
                         QueryHelper::Bindings() << category);
    QueryHelper::instance()->
        executeStatement("INSERT INTO category(name, icon, "
                         "visible, viewtype, viewsize) "
                         "VALUES(%s, %s, %s, %s, %s)",
                         QueryHelper::Bindings() << category << icon <<
                         size << type << showIt);

    emit categoryCollectionChanged();
}

#include "SQLCategoryCollection.moc"
