/* SPDX-FileCopyrightText: 2003-2020 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SelectionInteraction.h"

#include "CellGeometry.h"
#include "ThumbnailFactory.h"
#include "ThumbnailModel.h"
#include "ThumbnailWidget.h"

#include <MainWindow/Window.h>
#include <kpabase/FileNameList.h>

#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QUrl>

ThumbnailView::SelectionInteraction::SelectionInteraction(ThumbnailFactory *factory)
    : ThumbnailComponent(factory)
    , m_dragInProgress(false)
{
}

bool ThumbnailView::SelectionInteraction::mousePressEvent(QMouseEvent *event)
{
    m_mousePressPos = event->pos();
    const DB::FileName fileName = widget()->mediaIdUnderCursor();
    m_isMouseDragOperation = widget()->isSelected(fileName) && !event->modifiers();
    return m_isMouseDragOperation;
}

bool ThumbnailView::SelectionInteraction::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isMouseDragOperation) {
        if ((m_mousePressPos - event->pos()).manhattanLength() > QApplication::startDragDistance())
            startDrag();
        return true;
    }
    return false;
}

void ThumbnailView::SelectionInteraction::startDrag()
{
    m_dragInProgress = true;
    QList<QUrl> urls;
    const auto selection = widget()->selection(NoExpandCollapsedStacks);
    for (const DB::FileName &fileName : selection) {
        urls.append(QUrl::fromLocalFile(fileName.absolute()));
    }
    QDrag *drag = new QDrag(MainWindow::Window::theMainWindow());
    QMimeData *data = new QMimeData;
    data->setUrls(urls);
    drag->setMimeData(data);

    drag->exec(Qt::ActionMask);

    widget()->m_mouseHandler = &(widget()->m_mouseTrackingHandler);
    m_dragInProgress = false;
}

bool ThumbnailView::SelectionInteraction::isDragging() const
{
    return m_dragInProgress;
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_SelectionInteraction.cpp"
