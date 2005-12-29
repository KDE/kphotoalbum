#include "imagedb.h"
#include "XMLCategoryCollection.h"
#include "XMLCategory.h"

CategoryPtr XMLDB::XMLCategoryCollection::categoryForName( const QString& name ) const
{
    for( QValueList<CategoryPtr>::ConstIterator it = _categories.begin(); it != _categories.end(); ++it ) {
        if ( (*it)->name() == name )
            return *it;
    }
    return 0;
}

void XMLDB::XMLCategoryCollection::addCategory( Category* category )
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
    for( QValueList<CategoryPtr>::ConstIterator it = _categories.begin(); it != _categories.end(); ++it ) {
        res.append( (*it)->name() );
    }
    return res;
}

void XMLDB::XMLCategoryCollection::removeCategory( const QString& name )
{
    for( QValueList<CategoryPtr>::Iterator it = _categories.begin(); it != _categories.end(); ++it ) {
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
    ImageDB::instance()->renameCategory( oldName, newName );
    emit categoryCollectionChanged();

}

QValueList<CategoryPtr> XMLDB::XMLCategoryCollection::categories() const
{
    return _categories;
}

void XMLDB::XMLCategoryCollection::addCategory( const QString& text, const QString& icon, Category::ViewSize size, Category::ViewType type, bool show )
{
    addCategory( new XMLCategory( text, icon, size, type, show ) );
}

void XMLDB::XMLCategoryCollection::initIdMap()
{
    for( QValueList<CategoryPtr>::Iterator it = _categories.begin(); it != _categories.end(); ++it )
        static_cast<XMLCategory*>((*it).data())->initIdMap();
}

#include "XMLCategoryCollection.moc"
