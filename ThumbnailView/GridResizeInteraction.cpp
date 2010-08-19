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
#include "GridResizeInteraction.h"
#include "ThumbnailBuilder.h"
#include "ImageManager/ThumbnailCache.h"
#include "CellGeometry.h"
#include "ThumbnailModel.h"
#include <QScrollBar>
#include "ThumbnailWidget.h"
#include "Settings/SettingsData.h"
#include <KSharedConfig>
#include <KGlobal>

ThumbnailView::GridResizeInteraction::GridResizeInteraction( ThumbnailFactory* factory )
    : ThumbnailComponent( factory )
{
}

bool ThumbnailView::GridResizeInteraction::mousePressEvent( QMouseEvent* event )
{
    _resizing = true;
    _mousePressPos = event->pos();
    _origWidth = widget()->cellWidth();
    enterGridReziingMode();
    return true;
}


bool ThumbnailView::GridResizeInteraction::mouseMoveEvent( QMouseEvent* event )
{
    QPoint dist = event->pos() - _mousePressPos;

    Settings::SettingsData::instance()->setThumbSize( qMax( 32, _origWidth + dist.x()/5 ) );
    widget()->model()->reset();
    cellGeometryInfo()->calculateCellSize();
    return true;
}


bool ThumbnailView::GridResizeInteraction::mouseReleaseEvent( QMouseEvent* )
{
    _resizing = false;
    leaveGridResizingMode();
    return true;
}

bool ThumbnailView::GridResizeInteraction::isResizingGrid()
{
    return _resizing;
}


void ThumbnailView::GridResizeInteraction::leaveGridResizingMode()
{
    KGlobal::config()->sync();
    model()->reset();
    cellGeometryInfo()->flushCache();
    ImageManager::ThumbnailCache::instance()->flush();
    model()->updateVisibleRowInfo();
    widget()->setCurrentIndex( model()->index( m_currentRow, 0 ) );
    ThumbnailBuilder::instance()->buildAll();
}

void ThumbnailView::GridResizeInteraction::enterGridReziingMode()
{
    m_currentRow = widget()->currentIndex().row();
    widget()->verticalScrollBar()->setValue(0);
}

