#include "DB/ImageDB.h"
#include "XMLCategoryCollection.h"
#include "XMLCategory.h"

DB::CategoryPtr XMLDB::XMLCategoryCollection::categoryForName( const QString& name ) const
{
    for( QValueList<DB::CategoryPtr>::ConstIterator it = _categories.begin(); it != _categories.end(); ++it ) {
        if ( (*it)->name() == name )
            return *it;
    }
    return 0;
}

void XMLDB::XMLCategoryCollection::addCategory( DB::Category* category )
{
    _categories.append( category );
    connect( category, SIGNAL( changed() ), this, SIGNAL( categoryCollectionChanged() ) );
    connect( category, SIGNAL( itemRemoved( const QString& ) ), this, SLOT( itemRemoved( const QString& ) ) );
    connect( category, SIGNAL( itemRenamed( const QString&, const QString& ) ), this, SLOT( itemRenamed( const QString&, const QString& ) ) );
    emit categoryCollectionChanged();
}

QStringList XMLDB::XMLCategoryCollection::categoryNames() const
{
    QStringList res;
    for( QValueList<DB::CategoryPtr>::ConstIterator it = _categories.begin(); it != _categories.end(); ++it ) {
        res.append( (*it)->name() );
    }
    return res;
}

void XMLDB::XMLCategoryCollection::removeCategory( const QString& name )
{
    for( QValueList<DB::CategoryPtr>::Iterator it = _categories.begin(); it != _categories.end(); ++it ) {
        if ( (*it)->name() == name ) {
            _categories.remove(it);
            delete *it;
            emit categoryCollectionChanged();
            return;
        }
    }
    Q_ASSERT( false );
}

void XMLDB::XMLCategoryCollection::rename( const QString& oldName, const QString& newName )
{
    categoryForName(oldName)->setName(newName);
    DB::ImageDB::instance()->renameCategory( oldName, newName );
    emit categoryCollectionChanged();

}

QValueList<DB::CategoryPtr> XMLDB::XMLCategoryCollection::categories() const
{
    return _categories;
}

void XMLDB::XMLCategoryCollection::addCategory( const QString& text, const QString& icon, DB::Category::ViewSize size,
                                                DB::Category::ViewType type, int thumbnailSize, bool show )
{
    addCategory( new XMLCategory( text, icon, size, type, thumbnailSize, show ) );
}

void XMLDB::XMLCategoryCollection::initIdMap()
{
    for( QValueList<DB::CategoryPtr>::Iterator it = _categories.begin(); it != _categories.end(); ++it )
        static_cast<XMLCategory*>((*it).data())->initIdMap();
}

#include "XMLCategoryCollection.moc"
