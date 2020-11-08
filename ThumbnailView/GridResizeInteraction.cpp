/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "GridResizeInteraction.h"

#include "CellGeometry.h"
#include "ThumbnailModel.h"
#include "ThumbnailWidget.h"

#include <ImageManager/ThumbnailBuilder.h>
#include <ImageManager/ThumbnailCache.h>
#include <ImageManager/enums.h>
#include <MainWindow/Window.h>
#include <Settings/SettingsData.h>

#include <KLocalizedString>
#include <KSharedConfig>
#include <QScrollBar>
ThumbnailView::GridResizeInteraction::GridResizeInteraction(ThumbnailFactory *factory)
    : ThumbnailComponent(factory)
{
}

bool ThumbnailView::GridResizeInteraction::mousePressEvent(QMouseEvent *event)
{
    m_resizing = true;
    m_mousePressPos = event->pos();
    enterGridResizingMode();
    return true;
}

bool ThumbnailView::GridResizeInteraction::mouseMoveEvent(QMouseEvent *event)
{
    // no need to query this more than once (can't be changed in the GUI):
    static int _minimum_ = Settings::SettingsData::instance()->minimumThumbnailSize();
    QPoint dist = event->pos() - m_mousePressPos;
    setCellSize(qMax(_minimum_, m_origWidth + dist.x() / 5));
    return true;
}

bool ThumbnailView::GridResizeInteraction::mouseReleaseEvent(QMouseEvent *)
{
    leaveGridResizingMode();
    m_resizing = false;
    return true;
}

void ThumbnailView::GridResizeInteraction::setCellSize(int size)
{
    const int baseSize = Settings::SettingsData::instance()->thumbnailSize();

    // snap to base size:
    if (qAbs(size - baseSize) < 10)
        size = baseSize;

    model()->beginResetModel();
    Settings::SettingsData::instance()->setActualThumbnailSize(size);
    cellGeometryInfo()->calculateCellSize();
    model()->endResetModel();
}

bool ThumbnailView::GridResizeInteraction::isResizingGrid()
{
    return m_resizing;
}

void ThumbnailView::GridResizeInteraction::leaveGridResizingMode()
{
    KSharedConfig::openConfig()->sync();
    model()->beginResetModel();
    cellGeometryInfo()->flushCache();
    model()->endResetModel();
    model()->updateVisibleRowInfo();
    widget()->setCurrentIndex(model()->index(m_currentRow, 0));
}

void ThumbnailView::GridResizeInteraction::enterGridResizingMode()
{
    m_origWidth = widget()->cellWidth();
    ImageManager::ThumbnailBuilder::instance()->cancelRequests();
    m_currentRow = widget()->currentIndex().row();
    widget()->verticalScrollBar()->setValue(0);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
