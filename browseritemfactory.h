#ifndef BROWSERITEMFACTORY_H
#define BROWSERITEMFACTORY_H

class Folder;
#include <qlistview.h>
#include <qiconview.h>

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
    Folder* _folder;
};

class BrowserListItem :public QListViewItem
{
public:
    BrowserListItem( QListView* view, Folder* folder );
    Folder* _folder;
    virtual int compare( QListViewItem* other, int col, bool asc ) const;

};

#endif /* BROWSERITEMFACTORY_H */

