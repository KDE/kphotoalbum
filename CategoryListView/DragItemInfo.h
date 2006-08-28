#ifndef CATEGORYLISTVIEW_DRAGITEMINFO_H
#define CATEGORYLISTVIEW_DRAGITEMINFO_H

#include <qstring.h>
#include "Utilities/Set.h"

namespace CategoryListView
{

class DragItemInfo
{
public:
    DragItemInfo();
    DragItemInfo( const QString& parent, const QString& child );
    QString parent() const;
    QString child() const;
    void setParent( const QString& str );
    void setChild( const QString& str );
    bool operator<( const DragItemInfo& other ) const;

private:
    QString _parent;
    QString _child;
};


QDataStream& operator<<( QDataStream& stream, const DragItemInfo& );
QDataStream& operator>>( QDataStream& stream, DragItemInfo& );

typedef Set<DragItemInfo> DragItemInfoSet;
}

#endif /* CATEGORYLISTVIEW_DRAGITEMINFO_H */

