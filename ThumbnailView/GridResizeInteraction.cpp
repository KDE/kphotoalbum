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


