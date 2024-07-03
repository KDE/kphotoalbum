// SPDX-FileCopyrightText: 2003-2022 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2024 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "DragableTreeWidget.h"

#include "CheckDropItem.h"

#include <DB/Category.h>

#include <QDragMoveEvent>

CategoryListView::DragableTreeWidget::DragableTreeWidget(const DB::CategoryPtr &category, QWidget *parent)
    : QTreeWidget(parent)
    , m_category(category)
{
    setDragEnabled(true);
    setDragDropMode(DragDrop);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);
    setSelectionMode(ExtendedSelection);
}

DB::CategoryPtr CategoryListView::DragableTreeWidget::category() const
{
    return m_category;
}

void CategoryListView::DragableTreeWidget::emitItemsChanged()
{
    Q_EMIT itemsChanged();
}

QMimeData *CategoryListView::DragableTreeWidget::mimeData(const QList<QTreeWidgetItem *> items) const
{
    CategoryListView::DragItemInfoSet selected;
    for (QTreeWidgetItem *item : items) {
        QTreeWidgetItem *parent = item->parent();
        QString parentText = parent ? parent->text(0) : QString();
        selected.insert(CategoryListView::DragItemInfo(parentText, item->text(0)));
    }

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << selected;

    QMimeData *mime = new QMimeData;
    mime->setData(QString::fromUtf8("x-kphotoalbum/x-categorydrag"), data);
    return mime;
}

QStringList CategoryListView::DragableTreeWidget::mimeTypes() const
{
    return QStringList(QString::fromUtf8("x-kphotoalbum/x-categorydrag"));
}

bool CategoryListView::DragableTreeWidget::dropMimeData(QTreeWidgetItem *parent, int, const QMimeData *data, Qt::DropAction)
{
    CheckDropItem *targetItem = static_cast<CheckDropItem *>(parent);
    if (targetItem == nullptr) {
        // This can happen when an item is dropped between two other items and not
        // onto an item, which leads to a crash when calling dataDropped(data).
        return false;
    } else {
        return targetItem->dataDropped(data);
    }
}

void CategoryListView::DragableTreeWidget::dragMoveEvent(QDragMoveEvent *event)
{
    // Call super class in any case as it may scroll, which we want even if we reject
    QTreeWidget::dragMoveEvent(event);

    if (event->source() != this)
        event->ignore();

    QTreeWidgetItem *item = itemAt(event->position().toPoint());
    if (item && static_cast<CheckDropItem *>(item)->isSelfDrop(event->mimeData()))
        event->ignore();
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_DragableTreeWidget.cpp"
