#include "DragObject.h"
const char* CategoryListView::DragObject::format( int i ) const
{
    if ( i == 0 )
        return "x-kphotoalbum/x-category-drag";
    else
        return 0;
}

QByteArray CategoryListView::DragObject::encodedData( const char* ) const
{
    QByteArray res;
    QDataStream stream( res, IO_WriteOnly );
    stream << _items;
    return res;
}

CategoryListView::DragObject::DragObject( const CategoryListView::DragItemInfoSet& items, QWidget* dragSource )
    :QDragObject( dragSource ), _items( items )
{
}
