#include "SQLCategoryCollection.h"
#include "SQLCategory.h"
#include "QueryHelper.h"
#include "SQLFolderCategory.h"
#include "SQLSpecialCategory.h"
#include "QueryErrors.h"

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
    return QueryHelper::instance()->categoryNames();
}

void SQLDB::SQLCategoryCollection::removeCategory( const QString& name )
{
    try {
        QueryHelper::instance()->removeCategory(name);
    }
    catch (NotFoundError&) {
        return;
    }

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
    try {
        QueryHelper::instance()->
            insertCategory(category, icon, showIt, type, size);
    }
    catch (SQLError& e) {
        // Check if error occured, because category already exists
        try {
            QueryHelper::instance()->idForCategory(category);
        }
        catch (Error&) {
            throw e; // Throw the original error
        }
        return; // Don't overwrite existing category
    }
    emit categoryCollectionChanged();
}

#include "SQLCategoryCollection.moc"
