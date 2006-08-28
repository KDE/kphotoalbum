#ifndef DB_CATEGORYITEMS_H
#define DB_CATEGORYITEMS_H

#include <ksharedptr.h>
#include <qstring.h>
#include <qvaluelist.h>

namespace DB
{
class CategoryItem :public KShared
{
public:
    CategoryItem( const QString& name, bool isTop = false ) : _name( name ), _isTop( isTop ) {}
    ~CategoryItem();
    CategoryItem* clone() const;
    bool isDescendentOf( const QString& child, const QString& parent ) const;

protected:
    void print( int offset );
    bool hasChild( const QString& child );

public:
    QString _name;
    QValueList< CategoryItem* > _subcategories;
    bool _isTop;
};

}


#endif /* DB_CATEGORYITEMS_H */

