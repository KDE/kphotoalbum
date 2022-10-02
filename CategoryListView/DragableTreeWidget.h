// SPDX-FileCopyrightText: 2003-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

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

Q_SIGNALS:
    void itemsChanged();

private:
    const DB::CategoryPtr m_category;
};

}

#endif /* CATEGORYLISTVIEW_DragableTreeWidget_H */
// vi:expandtab:tabstop=4 shiftwidth=4:
