#include "DragItemInfo.h"

CategoryListView::DragItemInfo::DragItemInfo( const QString& parent, const QString& child )
    : _parent( parent ), _child( child )
{
}

QString CategoryListView::DragItemInfo::parent() const
{
    return _parent;
}

QString CategoryListView::DragItemInfo::child() const
{
    return _child;
}

bool CategoryListView::DragItemInfo::operator<( const DragItemInfo& other ) const
{
    return _parent < other._parent || (_parent == other._parent && _child < other._child );
}

CategoryListView::DragItemInfo::DragItemInfo()
{
}

QDataStream& CategoryListView::operator<<( QDataStream& stream, const DragItemInfo& info )
{
    stream << info.parent() << info.child();
    return stream;
}

QDataStream& CategoryListView::operator>>( QDataStream& stream, DragItemInfo& info )
{
    QString str;
    stream >> str;
    info.setParent( str );
    stream >> str;
    info.setChild( str );
    return stream;
}

void CategoryListView::DragItemInfo::setParent( const QString& str )
{
    _parent = str;
}

void CategoryListView::DragItemInfo::setChild( const QString& str )
{
    _child = str;
}
