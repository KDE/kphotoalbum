#include "SQLCategory.h"
#include "QueryHelper.h"

SQLDB::SQLCategory::SQLCategory(int categoryId):
    _categoryId(categoryId)
{
}

QString SQLDB::SQLCategory::name() const
{
    return QueryHelper::instance()->categoryForId(_categoryId);
}

void SQLDB::SQLCategory::setName(const QString& name)
{
    QueryHelper::instance()->
        executeStatement("UPDATE category SET name=%s WHERE id=%s",
                         QueryHelper::Bindings() << name << _categoryId);
}

QString SQLDB::SQLCategory::iconName() const
{
    return QueryHelper::instance()->
        executeQuery("SELECT icon FROM category WHERE id=%s",
                     QueryHelper::Bindings() << _categoryId).
        firstItem().toString();
}

void SQLDB::SQLCategory::setIconName( const QString& name )
{
    QueryHelper::instance()->
        executeStatement("UPDATE category SET icon=%s WHERE id=%s",
                         QueryHelper::Bindings() << name << _categoryId);
}

DB::Category::ViewSize SQLDB::SQLCategory::viewSize() const
{
    return static_cast<DB::Category::ViewSize>
        (QueryHelper::instance()->
         executeQuery("SELECT viewsize FROM category WHERE id=%s",
                      QueryHelper::Bindings() << _categoryId).
         firstItem().toInt());
}

void SQLDB::SQLCategory::setViewSize( ViewSize size )
{
    QueryHelper::instance()->
        executeStatement("UPDATE category SET viewsize=%s WHERE id=%s",
                         QueryHelper::Bindings() << size << _categoryId);
}

void SQLDB::SQLCategory::setViewType( ViewType type )
{
    QueryHelper::instance()->
        executeStatement("UPDATE category SET viewtype=%s WHERE id=%s",
                         QueryHelper::Bindings() << type << _categoryId);
}

DB::Category::ViewType SQLDB::SQLCategory::viewType() const
{
    return static_cast<DB::Category::ViewType>
        (QueryHelper::instance()->
         executeQuery("SELECT viewtype FROM category WHERE id=%s",
                      QueryHelper::Bindings() << _categoryId).
         firstItem().toInt());
}

void SQLDB::SQLCategory::setDoShow( bool b )
{
    QueryHelper::instance()->
        executeStatement("UPDATE category SET visible=%s WHERE id=%s",
                         QueryHelper::Bindings() << b << _categoryId);
}

bool SQLDB::SQLCategory::doShow() const
{
    return QueryHelper::instance()->
        executeQuery("SELECT visible FROM category WHERE id=%s",
                     QueryHelper::Bindings() << _categoryId).
        firstItem().toInt();
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
    QueryHelper::instance()->
        executeStatement("DELETE FROM tag WHERE categoryId=%s",
                         QueryHelper::Bindings() << _categoryId);

    uint place = items.count();
    for(QStringList::const_iterator it = items.begin();
        it != items.end(); ++it ) {
        QueryHelper::instance()->
            executeStatement("INSERT INTO tag(name, categoryId, place) "
                             "VALUES(%s, %s, %s)",
                             QueryHelper::Bindings() << *it <<
                             _categoryId << place);
        --place;
    }
}

void SQLDB::SQLCategory::removeItem(const QString& item)
{
    QueryHelper::instance()->removeTag(_categoryId, item);
}

void SQLDB::SQLCategory::renameItem(const QString& oldValue, const QString& newValue)
{
    QueryHelper::instance()->
        executeStatement("UPDATE tag SET name=%s "
                         "WHERE name=%s AND categoryId=%s",
                         QueryHelper::Bindings() <<
                         newValue << oldValue << _categoryId);
}

void SQLDB::SQLCategory::addItem( const QString& item )
{
    QueryHelper::instance()->insertTagFirst(_categoryId, item);
}

QStringList SQLDB::SQLCategory::items() const
{
    return QueryHelper::instance()->membersOfCategory(_categoryId);
}
