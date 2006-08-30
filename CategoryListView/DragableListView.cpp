#include "DragableListView.h"
#include "DragObject.h"
CategoryListView::DragableListView::DragableListView( const DB::CategoryPtr& category, QWidget* parent, const char* name )
    :QListView( parent, name ), _category( category )
{
}

QDragObject* CategoryListView::DragableListView::dragObject()
{
    CategoryListView::DragItemInfoSet selected;
    for ( QListViewItemIterator itemIt( this ); *itemIt; ++itemIt ) {
        if ( (*itemIt)->isSelected() ) {
            QListViewItem* parent = (*itemIt)->parent();
            QString parentText = parent ? parent->text(0) : QString::null;
            selected.insert( CategoryListView::DragItemInfo( parentText, (*itemIt)->text(0) ) );
        }
    }

    return new DragObject( selected, this );
}

DB::CategoryPtr CategoryListView::DragableListView::category() const
{
    return _category;
}

void CategoryListView::DragableListView::emitItemsChanged()
{
    emit itemsChanged();
}
