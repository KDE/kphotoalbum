#include "CategoryCollection.h"

using namespace DB;

void CategoryCollection::itemRenamed( const QString& oldName, const QString& newName )
{
    emit itemRenamed( static_cast<Category*>( const_cast<QObject*>( sender() ) ), oldName, newName );
}

void CategoryCollection::itemRemoved( const QString& item )
{
    emit itemRemoved( static_cast<Category*>( const_cast<QObject*>( sender() ) ), item );
}

/**
   See Category::text() for description why I might want to do this conversion.
*/
QString CategoryCollection::nameForText( const QString& text )
{
    QValueList<CategoryPtr> list = categories();
    for( QValueList<CategoryPtr>::Iterator it = list.begin(); it != list.end(); ++it ) {
        if ( (*it)->text() == text )
            return (*it)->name();
    }
    Q_ASSERT( false );
    return QString::null;
}

#include "CategoryCollection.moc"
