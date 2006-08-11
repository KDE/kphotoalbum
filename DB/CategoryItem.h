#ifndef DB_CATEGORYITEMS_H
#define DB_CATEGORYITEMS_H

#include <ksharedptr.h>

namespace DB
{
class CategoryItem :public KShared
{
public:
    CategoryItem( const QString& name, bool isTop = false ) : _name( name ), _isTop( isTop ) {}
    ~CategoryItem()
    {
        for( QValueList<CategoryItem*>::ConstIterator it = _subcategories.begin(); it != _subcategories.end(); ++it ) {
            delete *it;
        }
    }

    CategoryItem* clone()
    {
        CategoryItem* result = new CategoryItem( _name );
        for( QValueList<CategoryItem*>::ConstIterator it = _subcategories.begin(); it != _subcategories.end(); ++it ) {
            _subcategories.append( (*it)->clone() );
        }
        return result;
    }


    void print( int offset )
    {
        QString spaces;
        spaces.fill( ' ', offset );
        qDebug( "%s%s", spaces.latin1(), _name.latin1() );
        for( QValueList< CategoryItem* >::Iterator it = _subcategories.begin(); it != _subcategories.end(); ++it ) {
            (*it)->print( offset + 2 );
        }
    }

    QString _name;
    QValueList< CategoryItem* > _subcategories;
    bool _isTop;
};

}


#endif /* DB_CATEGORYITEMS_H */

