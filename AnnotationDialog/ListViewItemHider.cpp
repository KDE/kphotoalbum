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
 * \class AnnotationDialog::ListViewCheckedHider
 * \brief Helper class for only showing items that are selected.
 */

void AnnotationDialog::ListViewHider::setItemsVisible( QListView* listView )
{
    // It seems like a bug in Qt, but I need to make all item visible first, otherwise I see wrong items when I have items
    // two layers deep (A->B->c). It only occours when I widen (types "je", now deletes the "e" to get to "j" ).
    for ( QListViewItemIterator it( listView ); *it; ++it )
        (*it)->setVisible( true );

    for ( QListViewItem* item = listView->firstChild(); item; item = item->nextSibling() ) {
        bool anyChildrenVisible = setItemsVisible( item );
        bool visible = anyChildrenVisible || shouldItemBeShown( item );
        item->setVisible( visible );
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

AnnotationDialog::ListViewTextMatchHider::ListViewTextMatchHider( const QString& text, bool anchorAtStart, QListView* listView )
    :_text( text ), _anchorAtStart( anchorAtStart )
{
    setItemsVisible( listView );
}

bool AnnotationDialog::ListViewTextMatchHider::shouldItemBeShown( QListViewItem* item )
{
    if ( _anchorAtStart )
        return item->text(0).lower().startsWith( _text.lower() );
    else
        return item->text(0).lower().contains( _text.lower() );
}

bool AnnotationDialog::ListViewCheckedHider::shouldItemBeShown( QListViewItem* item )
{
    return static_cast<QCheckListItem*>(item)->isOn();
}

AnnotationDialog::ListViewCheckedHider::ListViewCheckedHider( QListView* listView )
{
    setItemsVisible( listView );
}
