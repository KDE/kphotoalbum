/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
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
