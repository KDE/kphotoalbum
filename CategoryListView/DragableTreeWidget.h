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
#ifndef CATEGORYLISTVIEW_DragableTreeWidget_H
#define CATEGORYLISTVIEW_DragableTreeWidget_H
#include <DB/CategoryPtr.h>

#include <QTreeWidget>

namespace CategoryListView
{
class DragableTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    DragableTreeWidget(const DB::CategoryPtr &category, QWidget *parent);
    DB::CategoryPtr category() const;
    void emitItemsChanged();

protected:
    QMimeData *mimeData(const QList<QTreeWidgetItem *> items) const override;
    QStringList mimeTypes() const override;
    bool dropMimeData(QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action) override;
    void dragMoveEvent(QDragMoveEvent *event) override;

signals:
    void itemsChanged();

private:
    const DB::CategoryPtr m_category;
};

}

#endif /* CATEGORYLISTVIEW_DragableTreeWidget_H */
// vi:expandtab:tabstop=4 shiftwidth=4:
