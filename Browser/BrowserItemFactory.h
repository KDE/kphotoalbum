/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef BROWSERITEMFACTORY_H
#define BROWSERITEMFACTORY_H

#include <qlistview.h>
#include <qiconview.h>

namespace Browser
{
class Folder;
class BrowserItem;
class BrowserListItem;

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
    virtual ~BrowserItemFactory() {}
    virtual BrowserItem* createItem( Folder*, BrowserItem* parentItem ) = 0;
    virtual bool supportsHierarchy() const = 0;
};


class BrowserIconViewItemFactory :public BrowserItemFactory
{
public:
    BrowserIconViewItemFactory( QIconView* view );
    virtual BrowserItem* createItem( Folder*, BrowserItem* parentItem );
    void setMatchText( const QString& text );
    virtual bool supportsHierarchy() const;

private:
    QIconView* _view;
    QString _matchText;
};


class BrowserListViewItemFactory :public BrowserItemFactory
{
public:
    BrowserListViewItemFactory( QListView* view );
    virtual BrowserItem* createItem( Folder*, BrowserItem* item );
    virtual bool supportsHierarchy() const;
private:
    QListView* _view;
};

class BrowserItem
{
public:
    virtual ~BrowserItem() {}
};

class BrowserIconItem :public QIconViewItem, public BrowserItem
{
public:
    BrowserIconItem( QIconView* view, Folder* folder );
    ~BrowserIconItem();
    Folder* _folder;
};


class BrowserListItem :public QListViewItem, public BrowserItem
{
public:
    BrowserListItem( QListView* view, Folder* folder );
    BrowserListItem( QListViewItem* view, Folder* folder );
    ~BrowserListItem();
    Folder* _folder;
    virtual int compare( QListViewItem* other, int col, bool asc ) const;
};

}

#endif /* BROWSERITEMFACTORY_H */

