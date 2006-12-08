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

