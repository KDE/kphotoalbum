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

