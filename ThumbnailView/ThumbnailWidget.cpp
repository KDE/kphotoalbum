/* Copyright (C) 2003-2009 Jesper K. Pedersen <blackie@kde.org>

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
#include "ThumbnailWidget.h"
#include "ThumbnailCache.h"
#include "ThumbnailDND.h"
#include "KeyboardEventHandler.h"
#include "ThumbnailFactory.h"
#include "ThumbnailModel.h"
#include "CellGeometry.h"
#include "ThumbnailWidget.moc"
#include "ThumbnailPainter.h"
#include <math.h>

#include <klocale.h>
#include <qcursor.h>
#include <qfontmetrics.h>
#include <qpainter.h>

#include "Browser/BrowserWidget.h"
#include "DB/ImageDB.h"
#include "DB/ImageInfoPtr.h"
#include "DB/ResultId.h"
#include "ImageManager/Manager.h"
#include "Settings/SettingsData.h"
#include "Utilities/Set.h"

/**
 * \class ThumbnailView::ThumbnailWidget
 * This is the widget which shows the thumbnails.
 *
 * In previous versions this was implemented using a QIconView, but there
 * simply was too many problems, so after years of tears and pains I
 * rewrote it.
 */
using Utilities::StringSet;

ThumbnailView::ThumbnailWidget::ThumbnailWidget( ThumbnailFactory* factory)
    :Q3GridView(),
     ThumbnailComponent( factory ),
     _isSettingDate(false),
     _gridResizeInteraction( this ),
     _wheelResizing( false ),
     _selectionInteraction( factory ),
     _mouseTrackingHandler( factory ),
     _mouseHandler( &_mouseTrackingHandler ),
     _dndHandler( new ThumbnailDND( factory ) ),
     _keyboardHandler( new KeyboardEventHandler( factory ) )
{
    setFocusPolicy( Qt::WheelFocus );
    updateCellSize();

    // It beats me why I need to set mouse tracking on both, but without it doesn't work.
    viewport()->setMouseTracking( true );
    setMouseTracking( true );

    connect( this, SIGNAL( contentsMoving( int, int ) ), this, SLOT( emitDateChange( int, int ) ) );
    connect( this, SIGNAL( contentsMoving( int, int ) ), this, SLOT( slotViewChanged( int, int ) ));
    viewport()->setAcceptDrops( true );

    setVScrollBarMode( AlwaysOn );
    setHScrollBarMode( AlwaysOff );

    connect( &_mouseTrackingHandler, SIGNAL( fileIdUnderCursorChanged( DB::ResultId ) ), this, SIGNAL( fileIdUnderCursorChanged( DB::ResultId ) ) );
    connect( _keyboardHandler, SIGNAL( showSelection() ), this, SIGNAL( showSelection() ) );
}

bool ThumbnailView::ThumbnailWidget::isGridResizing()
{
    return _mouseHandler->isResizingGrid() || _wheelResizing;
}

OVERRIDE void ThumbnailView::ThumbnailWidget::paintCell( QPainter * p, int row, int col )
{
    painter()->paintCell( p, row, col );
}

void ThumbnailView::ThumbnailWidget::generateMissingThumbnails(const DB::Result& items) const
{
    // TODO(hzeller) before release: run this asynchronously.
    //        For 100'000+ files, this will be slow.
    // jkt: this is dead slow even with 20k pictures on NFS
    // TODO-2(hzeller): This should be handled at startup or when new images
    //        enter the database; not here.
    // TODO-3(hzeller): With TODO-2 implemented, we probably don't need an
    //        existence cache anymore in ThumbnailStorage

    // Thumbnails are generated/stored in two sizes: 128x128 and 256x256.
    // Requesting the bigger one will make sure that both are stored.
    const QSize size(256, 256);
    ImageManager::Manager* imgManager = ImageManager::Manager::instance();
    Q_FOREACH(DB::ImageInfoPtr info, items.fetchInfos()) {
        const QString image = info->fileName(DB::AbsolutePath);
        if (imgManager->thumbnailsExist(image)) {
            continue;
        }
        ImageManager::ImageRequest* request
            = new ImageManager::ImageRequest(image, size, info->angle(), NULL);
        request->setPriority( ImageManager::BuildScopeThumbnails );
        imgManager->load( request );
    }
}

/** @short It seems that Q3GridView's viewportToContents() is slightly off */
QPoint ThumbnailView::ThumbnailWidget::viewportToContentsAdjusted( const QPoint& coordinate, CoordinateSystem system ) const
{
    QPoint contentsPos = coordinate;
    if ( system == ViewportCoordinates ) {
        contentsPos = viewportToContents( coordinate );
        contentsPos.rx() -= 3;
        contentsPos.ry() -= 3;
    }
    return contentsPos;
}



/**
 * Request a repaint of the cell showing filename
 *
 * QGridView::updateCell has some problems when we scroll during a keyboard selection, so thatswhy we role our own one.
 */
void ThumbnailView::ThumbnailWidget::updateCell( const DB::ResultId& id )
{
    if ( id.isNull() )
        return;

    painter()->repaint(id);
}

void ThumbnailView::ThumbnailWidget::updateCell( int row, int col )
{
    updateCell( model()->imageAt( row, col ) );
}


/**
 * Update the grid size depending on the size of the widget
 */
void ThumbnailView::ThumbnailWidget::updateGridSize()
{
    int thumbnailsPerRow = width() / cellWidth();
    int numRowsPerPage = height() / cellHeight();
    setNumCols( thumbnailsPerRow );
    setNumRows(qMax(numRowsPerPage,
                    static_cast<int>(
                        ceil(static_cast<double>(model()->imageCount()) / thumbnailsPerRow))));
    const QSize cellSize = cellGeometryInfo()->cellSize();
    const int border = Settings::SettingsData::instance()->thumbnailSpace();
    QSize thumbSize(cellSize.width() - 2 * border,
                    cellSize.height() - 2 * border);
    cache()->setThumbnailSize(thumbSize);
}

void ThumbnailView::ThumbnailWidget::showEvent( QShowEvent* )
{
    updateGridSize();
}

void ThumbnailView::ThumbnailWidget::keyPressEvent( QKeyEvent* event )
{
    _keyboardHandler->keyPressEvent( event );
}

void ThumbnailView::ThumbnailWidget::keyReleaseEvent( QKeyEvent* event )
{
    const bool propogate = _keyboardHandler->keyReleaseEvent( event );
    if ( propogate )
        Q3GridView::keyReleaseEvent(event);
}


/** @short Scroll the viewport so that the specified cell is visible */
void ThumbnailView::ThumbnailWidget::scrollToCell( const Cell& newPos )
{
    model()->setCurrentItem( newPos );

    // Scroll if necesary
    if ( newPos.row() > lastVisibleRow( FullyVisible ) )
        setContentsPos( contentsX(), cellGeometry( newPos.row(), newPos.col() ).top() -
                        (numRowsPerPage()-1)*cellHeight()  );

    if  ( newPos.row() < firstVisibleRow( FullyVisible ) )
        setContentsPos( contentsX(), cellGeometry( newPos.row(), newPos.col() ).top() );
}

/**
 * Return the number of complete rows per page
 */
int ThumbnailView::ThumbnailWidget::numRowsPerPage() const
{
    return height() / cellHeight();
}

bool ThumbnailView::ThumbnailWidget::isMouseOverStackIndicator( const QPoint& point )
{
    Cell pos = cellAtCoordinate( point, ViewportCoordinates );
    QRect cellRect = cellGeometry(pos.row(), pos.col() ).adjusted( 0, 0, -10, -10 ); // FIXME: what area should be "hot"?
    bool correctArea = !cellRect.contains( viewportToContentsAdjusted( point, ViewportCoordinates ) );
    if (!correctArea)
        return false;
    DB::ImageInfoPtr imageInfo = mediaIdUnderCursor().fetchInfo();
    return imageInfo && imageInfo->isStacked();
}

static bool isMouseResizeGesture( QMouseEvent* event )
{
    return
        (event->button() & Qt::MidButton) ||
        ((event->modifiers() & Qt::ControlModifier) && (event->modifiers() & Qt::AltModifier));
}

void ThumbnailView::ThumbnailWidget::mousePressEvent( QMouseEvent* event )
{
    if ( isMouseOverStackIndicator( event->pos() ) ) {
        model()->toggleStackExpansion( mediaIdUnderCursor() );
        return;
    }

    if ( isMouseResizeGesture( event ) )
        _mouseHandler = &_gridResizeInteraction;
    else
        _mouseHandler = &_selectionInteraction;

    _mouseHandler->mousePressEvent( event );
    
    if (event->button() & Qt::RightButton) //get out of selection mode if this is a right click
      _mouseHandler = &_mouseTrackingHandler;
      
}

void ThumbnailView::ThumbnailWidget::mouseMoveEvent( QMouseEvent* event )
{
    _mouseHandler->mouseMoveEvent( event );
}

void ThumbnailView::ThumbnailWidget::mouseReleaseEvent( QMouseEvent* event )
{
    _mouseHandler->mouseReleaseEvent( event );
    _mouseHandler = &_mouseTrackingHandler;
}

void ThumbnailView::ThumbnailWidget::mouseDoubleClickEvent( QMouseEvent * event )
{
    if ( isMouseOverStackIndicator( event->pos() ) ) {
        model()->toggleStackExpansion( mediaIdUnderCursor() );
    } else if ( !( event->modifiers() & Qt::ControlModifier ) ) {
        DB::ResultId id = model()->imageAt( event->pos(), ViewportCoordinates );
        if ( !id.isNull() )
            emit showImage( id );
    }
}

void ThumbnailView::ThumbnailWidget::wheelEvent( QWheelEvent* event )
{
    if ( event->modifiers() & Qt::ControlModifier ) {
        event->setAccepted(true);

        _wheelResizing = true;

        int delta = event->delta() / 20;

        Settings::SettingsData::instance()->setThumbSize( qMax( 32, cellWidth() + delta ) );

        updateCellSize();
    }
    else
        Q3GridView::wheelEvent(event);
}


void ThumbnailView::ThumbnailWidget::emitDateChange( int x, int y )
{
    if ( _isSettingDate )
        return;

    // Unfortunately the contentsMoving signal is emitted *before* the move, so we need to find out what is on the new position ourself.
    DB::ResultId id = model()->imageAt( rowAt(y), columnAt(x) );
    if ( id.isNull() )
        return;

    static QDateTime lastDate;
    QDateTime date = id.fetchInfo()->date().start();
    if ( date != lastDate ) {
        lastDate = date;
        if ( date.date().year() != 1900 )
            emit currentDateChanged( date );
    }
}

void ThumbnailView::ThumbnailWidget::slotViewChanged(int , int y) {
    if (isGridResizing())
        return;
    int startIndex = rowAt(y) * numCols();
    int endIndex = (rowAt( y + visibleHeight() ) + 1) * numCols();
    if (endIndex > model()->imageCount())
        endIndex = model()->imageCount();
    cache()->setHotArea(startIndex, endIndex);
}

/**
 * scroll to the date specified with the parameter date.
 * The boolean includeRanges tells whether we accept range matches or not.
 */
void ThumbnailView::ThumbnailWidget::gotoDate( const DB::ImageDate& date, bool includeRanges )
{
    _isSettingDate = true;
    DB::ResultId candidate = DB::ImageDB::instance()
                             ->findFirstItemInRange(model()->imageList(ViewOrder), date, includeRanges);
    if ( !candidate.isNull() ) {
        scrollToCell( model()->positionForMediaId( candidate ) );
        model()->setCurrentItem( candidate );
    }
    _isSettingDate = false;
}

/*
 * Returns the first row that is at least partly visible.
 */
int ThumbnailView::ThumbnailWidget::firstVisibleRow( VisibleState state ) const
{
    int firstRow = rowAt( contentsY() );
    if ( state == FullyVisible && rowAt( contentsY() + cellHeight()-1 ) != firstRow )
        firstRow += 1;

    return firstRow;
}

int ThumbnailView::ThumbnailWidget::lastVisibleRow( VisibleState state ) const
{
    int lastRow = rowAt( contentsY() + visibleHeight() );
    if ( state == FullyVisible && rowAt( contentsY() + visibleHeight() - cellHeight() -1 ) != lastRow )
        lastRow -= 1;
    return lastRow;
}

ThumbnailView::Cell ThumbnailView::ThumbnailWidget::cellAtCoordinate( const QPoint& pos, CoordinateSystem system ) const
{
    QPoint contentsPos = pos;
    if ( system == ViewportCoordinates )
        contentsPos = viewportToContentsAdjusted( pos, system );

    int col = columnAt( contentsPos.x() );
    int row = rowAt( contentsPos.y() );
    return Cell( row, col );
}


void ThumbnailView::ThumbnailWidget::resizeEvent( QResizeEvent* e )
{
    Q3GridView::resizeEvent( e );
    updateGridSize();
}


bool ThumbnailView::ThumbnailWidget::isFocusAtLastCell() const
{
    return model()->positionForMediaId(model()->currentItem() ) == lastCell();
}

bool ThumbnailView::ThumbnailWidget::isFocusAtFirstCell() const
{
    return model()->positionForMediaId(model()->currentItem()) == Cell(0,0);
}

/**
 * Return the coordinates of the last cell with a thumbnail in
 */
ThumbnailView::Cell ThumbnailView::ThumbnailWidget::lastCell() const
{
    return Cell((model()->imageCount() - 1) / numCols(),
                (model()->imageCount() - 1) % numCols());
}

void ThumbnailView::ThumbnailWidget::reload(bool flushCache, bool clearSelection)
{
    if ( flushCache )
        cache()->clear();
    if ( clearSelection )
        model()->clearSelection();
    updateCellSize();
    repaintScreen();
}

void ThumbnailView::ThumbnailWidget::repaintScreen()
{
    QPalette p;
    p.setColor( QPalette::Base, Settings::SettingsData::instance()->backgroundColor() );
    p.setColor( QPalette::Foreground, p.color(QPalette::WindowText ) );
    setPalette(p);

    const int first = firstVisibleRow( PartlyVisible );
    const int last = lastVisibleRow( PartlyVisible );
    for ( int row = first; row <= last; ++row )
        for ( int col = 0; col < numCols(); ++col )
            Q3GridView::repaintCell( row, col );
}

DB::ResultId ThumbnailView::ThumbnailWidget::mediaIdUnderCursor() const
{
    return model()->imageAt( mapFromGlobal( QCursor::pos() ), ViewportCoordinates );
}


void ThumbnailView::ThumbnailWidget::contentsDragMoveEvent( QDragMoveEvent* event )
{
    _dndHandler->contentsDragMoveEvent(event);
}

void ThumbnailView::ThumbnailWidget::contentsDragLeaveEvent( QDragLeaveEvent* event )
{
    _dndHandler->contentsDragLeaveEvent( event );
}

void ThumbnailView::ThumbnailWidget::contentsDropEvent( QDropEvent* event )
{
    _dndHandler->contentsDropEvent( event );
}


void ThumbnailView::ThumbnailWidget::dimensionChange( int oldNumRows, int /*oldNumCols*/ )
{
    if ( oldNumRows != numRows() )
        repaintScreen();
}

void ThumbnailView::ThumbnailWidget::updateCellSize()
{
    const QSize cellSize = cellGeometryInfo()->cellSize();
    setCellWidth( cellSize.width() );

    const int oldHeight = cellHeight();
    const int height = cellSize.height() + 2 + cellGeometryInfo()->textHeight( QFontMetrics( font() ).height(), true );
    setCellHeight( height );
    updateGridSize();
    if ( height != oldHeight && ! model()->currentItem().isNull() ) {
        const Cell c = model()->positionForMediaId(model()->currentItem());
        ensureCellVisible( c.row(), c.col() );
    }
}

void ThumbnailView::ThumbnailWidget::viewportPaintEvent( QPaintEvent* e )
{
    QPainter p( viewport() );
    p.fillRect( numCols() * cellWidth(), 0, width(), height(), palette().color(QPalette::Base) );
    p.fillRect( 0, numRows() * cellHeight(), width(), height(), palette().color(QPalette::Base) );
    p.end();
    Q3GridView::viewportPaintEvent( e );
}

void ThumbnailView::ThumbnailWidget::contentsDragEnterEvent( QDragEnterEvent * event )
{
    _dndHandler->contentsDragEnterEvent( event );
}

