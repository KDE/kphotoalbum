// SPDX-FileCopyrightText: 2003-2022 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2003-2025 The KPhotoAlbum Development Team
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "MouseTrackingInteraction.h"

#include "ThumbnailModel.h"
#include "ThumbnailWidget.h"
#include "VideoThumbnailCycler.h"

#include <kpabase/FileName.h>

#include <QMouseEvent>

ThumbnailView::MouseTrackingInteraction::MouseTrackingInteraction(ThumbnailFactory *factory)
    : ThumbnailComponent(factory)
    , m_cursorWasAtStackIcon(false)
{
}

bool ThumbnailView::MouseTrackingInteraction::mouseMoveEvent(QMouseEvent *event)
{
    updateStackingIndication(event);
    handleCursorOverNewIcon();

    if ((event->modifiers() & Qt::ControlModifier) != 0 && widget()->isItemUnderCursorSelected())
        VideoThumbnailCycler::instance()->stopCycle();
    else
        VideoThumbnailCycler::instance()->setActive(widget()->mediaIdUnderCursor());
    return false;
}

void ThumbnailView::MouseTrackingInteraction::updateStackingIndication(QMouseEvent *event)
{
    bool interestingArea = widget()->isMouseOverStackIndicator(event->pos());
    if (interestingArea && !m_cursorWasAtStackIcon) {
        widget()->setCursor(Qt::PointingHandCursor);
        m_cursorWasAtStackIcon = true;
    } else if (!interestingArea && m_cursorWasAtStackIcon) {
        widget()->unsetCursor();
        m_cursorWasAtStackIcon = false;
    }
}

void ThumbnailView::MouseTrackingInteraction::handleCursorOverNewIcon()
{
    static DB::FileName lastFileNameUnderCursor;
    const DB::FileName fileName = widget()->mediaIdUnderCursor();

    if (fileName != lastFileNameUnderCursor) {
        if (!fileName.isNull() && !lastFileNameUnderCursor.isNull()) {
            Q_EMIT fileIdUnderCursorChanged(fileName);
            const QModelIndex lastIndex = model()->fileNameToIndex(lastFileNameUnderCursor);
            if (lastIndex.isValid()) {
                // The index is invalid if it refers to a file that is
                // not in the current view (eg. when the view changes).
                model()->updateCell(lastIndex);
            }
            model()->updateCell(fileName);
        }
        lastFileNameUnderCursor = fileName;
    }
}
// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_MouseTrackingInteraction.cpp"
