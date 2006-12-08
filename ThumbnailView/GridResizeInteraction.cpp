/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "GridResizeInteraction.h"
#include "ThumbnailWidget.h"
#include "Settings/SettingsData.h"

ThumbnailView::GridResizeInteraction::GridResizeInteraction( ThumbnailWidget* view )
    : _view( view )
{
}

void ThumbnailView::GridResizeInteraction::mousePressEvent( QMouseEvent* event )
{
    _resizing = true;
    _mousePressPos = event->pos();
    _view->setContentsPos( 0, 0 );
    _origSize = QSize( _view->cellWidth(), _view->cellHeight() );
}


void ThumbnailView::GridResizeInteraction::mouseMoveEvent( QMouseEvent* event )
{
    QPoint dist = event->pos() - _mousePressPos;
    int h = 0;
    if ( Settings::SettingsData::instance()->displayLabels() )
        h = QFontMetrics( _view->font() ).height();

    _view->setCellWidth( QMAX( 32, _origSize.width() + (dist.x() + dist.y())/10 ) );
    _view->setCellHeight( QMAX( 32 + h, _origSize.height() + (dist.x() + dist.y())/10 ) );

    _view->updateGridSize();
}


void ThumbnailView::GridResizeInteraction::mouseReleaseEvent( QMouseEvent* )
{
    int delta = _view->cellWidth() - _origSize.width();
    Settings::SettingsData::instance()->setThumbSize( Settings::SettingsData::instance()->thumbSize() + delta);
    if ( !_view->_currentItem.isNull() ) {
        Cell cell = _view->positionForFileName( _view->_currentItem );
        _view->ensureCellVisible( cell.row(), cell.col() );
    }
    _resizing = false;
    _view->repaintScreen();
}

bool ThumbnailView::GridResizeInteraction::isResizingGrid()
{
    return _resizing;
}


