#include "categorycollection.h"
#include "imagedb.h"

CategoryCollection* CategoryCollection::_instance = 0;

CategoryCollection::CategoryCollection()
    :QObject( 0, "CategoryCollection" )
{
}

CategoryCollection* CategoryCollection::instance()
{
    if ( !_instance )
        _instance = new CategoryCollection;
    return _instance;
}

Category* CategoryCollection::categoryForName( const QString& name )
{
    for( QValueList<Category*>::Iterator it = _categories.begin(); it != _categories.end(); ++it ) {
        if ( (*it)->name() == name )
            return *it;
    }
    return 0;
}

void CategoryCollection::addCategory( Category* category )
{
    _categories.append( category );
    connect( category, SIGNAL( changed() ), this, SIGNAL( categoryCollectionChanged() ) );
    emit categoryCollectionChanged();
}

QStringList CategoryCollection::categoryNames()
{
    QStringList res;
    for( QValueList<Category*>::Iterator it = _categories.begin(); it != _categories.end(); ++it ) {
        res.append( (*it)->name() );
    }
    return res;
}

void CategoryCollection::removeCategory( const QString& name )
{
    for( QValueList<Category*>::Iterator it = _categories.begin(); it != _categories.end(); ++it ) {
        if ( (*it)->name() == name ) {
            _categories.remove(it);
            delete *it;
            emit categoryCollectionChanged();
            return;
        }
    }
    Q_ASSERT( false );
}

void CategoryCollection::rename( const QString& oldName, const QString& newName )
{
    categoryForName(oldName)->setName(newName);
    Options::instance()->_options[newName] = Options::instance()->_options[oldName];
    Options::instance()->_options.erase(oldName);
    ImageDB::instance()->renameOptionGroup( oldName, newName );
    emit categoryCollectionChanged();

}

const QValueList<Category*>& CategoryCollection::categories() const
{
    return _categories;
}

/**
   See Category::text() for description why I might want to do this conversion.
*/
QString CategoryCollection::nameForText( const QString& text )
{
    for( QValueList<Category*>::Iterator it = _categories.begin(); it != _categories.end(); ++it ) {
        if ( (*it)->text() == text )
            return (*it)->name();
    }
    Q_ASSERT( false );
    return QString::null;
}


#include "categorycollection.moc"
