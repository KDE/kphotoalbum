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

#include <q3listview.h>
#include <q3iconview.h>

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
    BrowserIconViewItemFactory( Q3IconView* view );
    virtual BrowserItem* createItem( Folder*, BrowserItem* parentItem );
    void setMatchText( const QString& text );
    virtual bool supportsHierarchy() const;

private:
    Q3IconView* _view;
    QString _matchText;
};


class BrowserListViewItemFactory :public BrowserItemFactory
{
public:
    BrowserListViewItemFactory( Q3ListView* view );
    virtual BrowserItem* createItem( Folder*, BrowserItem* item );
    virtual bool supportsHierarchy() const;
private:
    Q3ListView* _view;
};

class BrowserItem
{
public:
    virtual ~BrowserItem() {}
};

class BrowserIconItem :public Q3IconViewItem, public BrowserItem
{
public:
    BrowserIconItem( Q3IconView* view, Folder* folder );
    ~BrowserIconItem();
    Folder* _folder;
};


class BrowserListItem :public Q3ListViewItem, public BrowserItem
{
public:
    BrowserListItem( Q3ListView* view, Folder* folder );
    BrowserListItem( Q3ListViewItem* view, Folder* folder );
    ~BrowserListItem();
    Folder* _folder;
    virtual int compare( Q3ListViewItem* other, int col, bool asc ) const;
};

}

#endif /* BROWSERITEMFACTORY_H */

