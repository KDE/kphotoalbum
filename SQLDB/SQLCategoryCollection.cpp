#include "SQLCategoryCollection.h"
#include "QueryHelper.h"
#include "SQLNormalCategory.h"
#include "SQLTokensCategory.h"
#include "SQLFolderCategory.h"
#include "QueryErrors.h"

SQLDB::SQLCategoryCollection::SQLCategoryCollection(Connection& connection):
    _connection(&connection),
    _qh(connection)
{
    // Categories, which should not have an entry in category table.
    _specialCategoryNames << "Folder";
}


DB::CategoryPtr SQLDB::SQLCategoryCollection::categoryForName( const QString& name ) const
{
    DB::CategoryPtr p;

    if (name == "Folder") {
        p = new SQLFolderCategory(const_cast<QueryHelper*>(&_qh));
    }
    else {
        int categoryId;
        try {
            categoryId = _qh.categoryId(name);
        }
        catch (NotFoundError&) {
            return 0;
        }
        if (name == "Tokens") {
            p = new SQLTokensCategory(const_cast<QueryHelper*>(&_qh), categoryId);
        }
        else {
            p = new SQLNormalCategory(const_cast<QueryHelper*>(&_qh), categoryId);
        }
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
    return _specialCategoryNames + _qh.categoryNames();
}

void SQLDB::SQLCategoryCollection::removeCategory( const QString& name )
{
    if (_specialCategoryNames.contains(name))
        return;

    try {
        _qh.removeCategory(name);
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

void SQLDB::SQLCategoryCollection::addCategory(const QString& category,
                                               const QString& icon,
                                               DB::Category::ViewType type,
                                               int thumbnailSize, bool showIt)
{
    if (_specialCategoryNames.contains(category))
        return;

    try {
        _qh.insertCategory(category, icon, showIt, type, thumbnailSize);
    }
    catch (SQLError& e) {
        // Check if error occured, because category already exists
        try {
            _qh.categoryId(category);
        }
        catch (Error&) {
            throw e; // Throw the original error
        }
        return; // Don't overwrite existing category
    }
    emit categoryCollectionChanged();
}

#include "SQLCategoryCollection.moc"
