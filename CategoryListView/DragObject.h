#ifndef CATEGORYLISTVIEW_DRAGOBJECT_H
#define CATEGORYLISTVIEW_DRAGOBJECT_H

#include <qdragobject.h>
#include <qstringlist.h>
#include "DragItemInfo.h"

class QListViewItem;

namespace CategoryListView
{
class DragObject :public QDragObject
{
public:
    DragObject( const DragItemInfoSet&, QWidget* dragSource );
    virtual const char* format ( int i = 0 ) const;
    virtual QByteArray encodedData ( const char* ) const;

private:
    DragItemInfoSet _items;
};

}

#endif /* CATEGORYLISTVIEW_DRAGOBJECT_H */

