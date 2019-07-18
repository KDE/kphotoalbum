/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef CATEGORYLISTVIEW_CHECKDROPITEM_H
#define CATEGORYLISTVIEW_CHECKDROPITEM_H

#include "DragItemInfo.h"
#include <QMimeData>
#include <QTreeWidgetItem>

class QDropEvent;
namespace CategoryListView
{
class DragableTreeWidget;

/*
 * Implementation detail:
 * The drag and drop support here is partly similar to Browser::TreeCategoryModel.
 * Any bugs there probably apply here as well and vice versa.
 */
class CheckDropItem : public QTreeWidgetItem
{
public:
    CheckDropItem(DragableTreeWidget *listview, const QString &column1, const QString &column2);
    CheckDropItem(DragableTreeWidget *listview, QTreeWidgetItem *parent, const QString &column1, const QString &column2);
    void setDNDEnabled(bool);
    bool dataDropped(const QMimeData *data);
    bool isSelfDrop(const QMimeData *data) const;
    void setTristate(bool b);

protected:
    bool verifyDropWasIntended(const QString &parent, const DragItemInfoSet &children);
    DragItemInfoSet extractData(const QMimeData *data) const;

private:
    DragableTreeWidget *m_listView;
};

}

#endif /* CATEGORYLISTVIEW_CHECKDROPITEM_H */
// vi:expandtab:tabstop=4 shiftwidth=4:
