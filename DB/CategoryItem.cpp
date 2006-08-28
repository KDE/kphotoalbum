#include "CategoryItem.h"
DB::CategoryItem::~CategoryItem()
{
    for( QValueList<CategoryItem*>::ConstIterator it = _subcategories.begin(); it != _subcategories.end(); ++it ) {
        delete *it;
    }
}

DB::CategoryItem* DB::CategoryItem::clone() const
{
    CategoryItem* result = new CategoryItem( _name );
    for( QValueList<CategoryItem*>::ConstIterator it = _subcategories.begin(); it != _subcategories.end(); ++it ) {
        result->_subcategories.append( (*it)->clone() );
    }
    return result;
}

void DB::CategoryItem::print( int offset )
{
    QString spaces;
    spaces.fill( ' ', offset );
    qDebug( "%s%s", spaces.latin1(), _name.latin1() );
    for( QValueList< CategoryItem* >::Iterator it = _subcategories.begin(); it != _subcategories.end(); ++it ) {
        (*it)->print( offset + 2 );
    }
}

bool DB::CategoryItem::isDescendentOf( const QString& child, const QString& parent ) const
{
   for( QValueList< CategoryItem* >::ConstIterator it = _subcategories.begin(); it != _subcategories.end(); ++it ) {
        if ( _name == parent ) {
            if ( (*it)->hasChild( child ) )
                return true;
        }
        else {
            if ( (*it)->isDescendentOf( child, parent ) )
                return true;
        }
    }
    return false;
}

bool DB::CategoryItem::hasChild( const QString& child )
{
    if ( _name == child )
        return true;

    for( QValueList< CategoryItem* >::Iterator it = _subcategories.begin(); it != _subcategories.end(); ++it ) {
        if ( (*it)->hasChild( child ) )
            return true;
    }
    return false;
}



