#ifndef CATEGORYLISTVIEW_CHECKDROPITEM_H
#define CATEGORYLISTVIEW_CHECKDROPITEM_H

#include <qlistview.h>
#include "DragItemInfo.h"
#include "DB/CategoryItem.h"
#include <ksharedptr.h>

namespace CategoryListView
{
class DragableListView;

class CheckDropItem :public QCheckListItem
{
public:
    CheckDropItem( DragableListView* listview, const QString& column1, const QString& column2 );
    CheckDropItem( DragableListView* listview, QListViewItem* parent, const QString& column1, const QString& column2 );
    virtual bool acceptDrop( const QMimeSource* mime ) const;
    void setDNDEnabled( bool );

protected:
    virtual void dropped( QDropEvent* e );
    bool isSelfDrop( const QString& parent, const DragItemInfoSet& children ) const;
    bool verifyDropWasIntended( const QString& parent, const DragItemInfoSet& children );
    DragItemInfoSet extractData( const QMimeSource* ) const;
    virtual void activate();

private:
    DragableListView* _listView;
};

}

#endif /* CATEGORYLISTVIEW_CHECKDROPITEM_H */

