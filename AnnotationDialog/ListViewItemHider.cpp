#include "ListViewItemHider.h"

/**
 * \class AnnotationDialog::ListViewHider
 * \brief Helper class, used to hide/show listview items
 *
 * This is a helper class that is used to hide items in a listview. A leaf
 * will be hidden if then subclass implemented method \ref
 * shouldItemBeShown returns true for the given item. A parent node is
 * hidden if none of the children are shown, and \ref shouldItemBeShown
 * also returns false for the parent itself.
 */

/**
 * \class AnnotationDialog::ListViewTextMatchHider
 * \brief Helper class for showing items matching a given text string.
 */

/**
 * \class AnnotationDialog::ListViewSelectionHider
 * \brief Helper class for only showing items that are selected.
 */

void AnnotationDialog::ListViewHider::setItemsVisible( QListView* listView )
{
    for ( QListViewItem* item = listView->firstChild(); item; item = item->nextSibling() ) {
        bool anyChildrenVisible = setItemsVisible( item );
        item->setVisible( anyChildrenVisible || shouldItemBeShown( item ) );
    }
}

bool AnnotationDialog::ListViewHider::setItemsVisible( QListViewItem* parentItem )
{
    bool anyChildrenVisible = false;
    for ( QListViewItem* item = parentItem->firstChild(); item; item = item->nextSibling() ) {
        bool anySubChildrenVisible = setItemsVisible( item );
        bool itemVisible = anySubChildrenVisible || shouldItemBeShown( item );
        item->setVisible( itemVisible );
        anyChildrenVisible |= itemVisible;
    }
    return anyChildrenVisible;
}

AnnotationDialog::ListViewTextMatchHider::ListViewTextMatchHider( const QString& text, QListView* listView )
    :_text( text )
{
    setItemsVisible( listView );
}

bool AnnotationDialog::ListViewTextMatchHider::shouldItemBeShown( QListViewItem* item )
{
    return item->text(0).lower().startsWith( _text.lower() );
}

bool AnnotationDialog::ListViewSelectionHider::shouldItemBeShown( QListViewItem* item )
{
    return item->isSelected();
}

AnnotationDialog::ListViewSelectionHider::ListViewSelectionHider( QListView* listView )
{
    setItemsVisible( listView );
}
