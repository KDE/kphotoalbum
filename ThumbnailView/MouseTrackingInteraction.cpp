#include "MouseTrackingInteraction.h"
#include "ThumbnailView.h"

ThumbnailView::MouseTrackingInteraction::MouseTrackingInteraction( ThumbnailView* view )
    :_view( view )
{
}


void ThumbnailView::MouseTrackingInteraction::mouseMoveEvent( QMouseEvent* event )
{
    static QString lastFileNameUderCursor;
    QString fileName = _view->fileNameAtViewportPos( event->pos() );
    if ( fileName != lastFileNameUderCursor ) {
        emit _view->fileNameUnderCursorChanged( fileName );
        lastFileNameUderCursor = fileName;
    }
}
