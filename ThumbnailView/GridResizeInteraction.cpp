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
#include "ImageManager/ThumbnailBuilder.h"
#include "ImageManager/ThumbnailCache.h"
#include "CellGeometry.h"
#include "ThumbnailModel.h"
#include <QScrollBar>
#include "ThumbnailWidget.h"
#include "Settings/SettingsData.h"
#include <KSharedConfig>
#include <KGlobal>
#include "MainWindow/Window.h"
#include <klocale.h>
#include <KMessageBox>
#include "ImageManager/enums.h"
ThumbnailView::GridResizeInteraction::GridResizeInteraction( ThumbnailFactory* factory )
    : ThumbnailComponent( factory )
{
}

bool ThumbnailView::GridResizeInteraction::mousePressEvent( QMouseEvent* event )
{
    _resizing = true;
    _mousePressPos = event->pos();
    enterGridResizingMode();
    return true;
}


bool ThumbnailView::GridResizeInteraction::mouseMoveEvent( QMouseEvent* event )
{
    QPoint dist = event->pos() - _mousePressPos;
    setCellSize( qMax( 32, _origWidth + dist.x()/5 ) );
    return true;
}


bool ThumbnailView::GridResizeInteraction::mouseReleaseEvent( QMouseEvent* )
{
    leaveGridResizingMode();
    _resizing = false;
    return true;
}

void ThumbnailView::GridResizeInteraction::setCellSize(int size)
{
    Settings::SettingsData::instance()->setThumbSize( size );
    model()->reset();
    cellGeometryInfo()->calculateCellSize();
}


bool ThumbnailView::GridResizeInteraction::isResizingGrid()
{
    return _resizing;
}


void ThumbnailView::GridResizeInteraction::leaveGridResizingMode()
{
    int code =  KMessageBox::questionYesNo( MainWindow::Window::theMainWindow(),
                                            i18n("Really resize grid, it will result in all thumbnails being regenerated?"),
                                            i18n("Really resize grid?"),
                                            KStandardGuiItem::yes(), KStandardGuiItem::no(),
                                            QLatin1String("resizeGrid"));
    if ( code == KMessageBox::Yes ) {
        KGlobal::config()->sync();
        model()->reset();
        cellGeometryInfo()->flushCache();
        ImageManager::ThumbnailCache::instance()->flush();
        model()->updateVisibleRowInfo();
        widget()->setCurrentIndex( model()->index( m_currentRow, 0 ) );
        ImageManager::ThumbnailBuilder::instance()->buildAll( ImageManager::StartDelayed );
    }
    else
        setCellSize( _origWidth );
}

void ThumbnailView::GridResizeInteraction::enterGridResizingMode()
{
    _origWidth = widget()->cellWidth();
    ImageManager::ThumbnailBuilder::instance()->cancelRequests();
    m_currentRow = widget()->currentIndex().row();
    widget()->verticalScrollBar()->setValue(0);
}

