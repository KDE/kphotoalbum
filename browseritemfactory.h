#ifndef BROWSERITEMFACTORY_H
#define BROWSERITEMFACTORY_H

class Folder;
#include <qlistview.h>
#include <qiconview.h>

/**
   The Browser can display the items either in an QIconView or a QListView.
   The original design of the browser was very generic, and thus without
   any information about what was shown in the browser. This had the
   advantage that back and forth was very simply to
   implement. Unfortunately it made implementing the possibility of showing
   either an IconView or a ListView a bit more difficult.

   Here is the current design.
   The class BrowserItemFactory is inherited by a factory for generating
   QListViewItems (BrowserListViewItemFactory) and one for generating
   QIconViewItem (BrowserIconViewItemFactory).
   Doing so, each of the Folder classes do not need to worry about whether
   they are populating an IconView or a ListView.

   Items in the IconView are instances of the class BrowserIconItem, while
   items in the ListView are instances of the class BrowserListItem.
   Both classes contains a pointer to the Folder data structure, which
   contains the actual information about the item, plus the capability of
   populating the view, when the given item is selected.
*/
class BrowserItemFactory
{
public:
    BrowserItemFactory() {}
    virtual void createItem( Folder* ) = 0;
};


class BrowserIconViewItemFactory :public BrowserItemFactory
{
public:
    BrowserIconViewItemFactory( QIconView* view );
    virtual void createItem( Folder* );
private:
    QIconView* _view;
};


class BrowserListViewItemFactory :public BrowserItemFactory
{
public:
    BrowserListViewItemFactory( QListView* view );
    virtual void createItem( Folder* );
private:
    QListView* _view;
};


class BrowserIconItem :public QIconViewItem
{
public:
    BrowserIconItem( QIconView* view, Folder* folder );
    ~BrowserIconItem();
    Folder* _folder;
};


class BrowserListItem :public QListViewItem
{
public:
    BrowserListItem( QListView* view, Folder* folder );
    ~BrowserListItem();
    Folder* _folder;
    virtual int compare( QListViewItem* other, int col, bool asc ) const;

};

#endif /* BROWSERITEMFACTORY_H */

