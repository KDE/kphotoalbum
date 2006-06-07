#include "ThumbnailWidget.h"
#include <qpixmapcache.h>
#include <qpainter.h>
#include "math.h"
#include "Settings/SettingsData.h"
#include "ThumbnailRequest.h"
#include "ImageManager/Manager.h"
#include "DB/ImageInfoPtr.h"
#include "DB/ImageDB.h"
#include "Utilities/Util.h"
#include <qcursor.h>
#include <qapplication.h>
#include "ThumbnailToolTip.h"
#include "Browser/BrowserWidget.h"
#include <klocale.h>
#include <kmessagebox.h>
#include <qfontmetrics.h>
#include "ThumbnailWidget.moc"

/**
 * \namespace ThumbnailView
 * The thumbnail view.
 */

/**
 * \class ThumbnailView
 * This is the widget which shows the thumbnails.
 *
 * In previous versions this was implemented using a QIconView, but there
 * simply was too many problems, so after years of tears and pains I
 * rewrote it.
 */

ThumbnailView::ThumbnailWidget* ThumbnailView::ThumbnailWidget::_instance = 0;
ThumbnailView::ThumbnailWidget::ThumbnailWidget( QWidget* parent, const char* name )
    :QGridView( parent, name ),
     _gridResizeInteraction( this ),
     _selectionInteraction( this ),
     _mouseTrackingHandler( this ),
     _mouseHandler( &_mouseTrackingHandler ),
     _sortDirection( Settings::SettingsData::instance()->showNewestThumbnailFirst() ? NewestFirst : OldestFirst )
{
    _instance = this;
    setFocusPolicy( WheelFocus );
    updateCellSize();

    // It beats me why I need to set mouse tracking on both, but without it doesn't work.
    viewport()->setMouseTracking( true );
    setMouseTracking( true );

    connect( this, SIGNAL( contentsMoving( int, int ) ), this, SLOT( emitDateChange( int, int ) ) );

    _toolTip = new ThumbnailToolTip( this );

    viewport()->setAcceptDrops( true );

    _repaintTimer = new QTimer( this );
    connect( _repaintTimer, SIGNAL( timeout() ), this, SLOT( slotRepaint() ) );

    viewport()->setBackgroundMode( NoBackground );

    connect( this, SIGNAL( contentsMoving( int, int ) ), this, SLOT( ensureCurrentVisible() ) );

    setVScrollBarMode( AlwaysOn );
    setHScrollBarMode( AlwaysOff );
}


void ThumbnailView::ThumbnailWidget::paintCell( QPainter * p, int row, int col )
{
    QPixmap doubleBuffer( cellRect().size() );
    QPainter painter( &doubleBuffer );
    paintCellBackground( &painter, row, col );
    if ( !_mouseHandler->isResizingGrid() ) {
        paintCellPixmap( &painter, row, col );
        paintCellText( &painter, row, col );
    }
    painter.end();
    p->drawPixmap( cellRect(), doubleBuffer );
}

/**
 * Paint the pixmap in the cell (row,col)
 */
void ThumbnailView::ThumbnailWidget::paintCellPixmap( QPainter* painter, int row, int col )
{
    QString fileName = fileNameInCell( row, col );
    if ( !fileName.isNull() ) {
        QPixmap* pix = QPixmapCache::find( fileNameInCell( row, col ) );
        if ( pix ) {
            QRect rect = iconGeometry( row, col );
            Q_ASSERT( !rect.isNull() );
            painter->drawPixmap( rect, *pix );
            if ( _selectedFiles.contains( fileName ) )
                painter->fillRect( rect, QBrush( palette().active().highlight(), Dense4Pattern ) );

            rect = QRect( 0, 0, cellWidth(), cellHeight() );
            if ( _leftDrop == fileName )
                painter->fillRect( rect.left(), rect.top(), 3, rect.height(), QBrush( red ) );
            else if ( _rightDrop == fileName )
                painter->fillRect( rect.right() -2, rect.top(), 3, rect.height(), QBrush( red ) );

        }
        else {
            int size = Settings::SettingsData::instance()->thumbSize();
            int angle = DB::ImageDB::instance()->info( fileName )->angle();
            ThumbnailRequest* request = new ThumbnailRequest( fileName, QSize( size, size ), angle, this );
            request->setCache();
            ImageManager::Manager::instance()->load( request );
        }
    }
}

/**
 * Draw the title under the thumbnail
 */
void ThumbnailView::ThumbnailWidget::paintCellText( QPainter* painter, int row, int col )
{
    if ( !Settings::SettingsData::instance()->displayLabels() )
        return;

    QString fileName = fileNameInCell( row, col );
    if ( fileName.isNull() )
        return;

    QString title = DB::ImageDB::instance()->info( fileName )->label();
    QRect rect = cellTextGeometry( row, col );
    painter->setPen( palette().active().text() );

    int align = (QFontMetrics( font() ).width( title ) > rect.width()) ? Qt::AlignLeft : Qt::AlignCenter;

    painter->drawText( rect, align, title );
}



void ThumbnailView::ThumbnailWidget::setImageList( const QStringList& list )
{
    QStringList l;
    if ( _sortDirection == OldestFirst )
        l = list;
    else
        l = reverseList( list );

    _imageList.clear();
    _imageList.reserve( list.count() );
    for( QStringList::ConstIterator it = l.begin(); it != l.end(); ++it ) {
        _imageList.append( *it );
    }
    updateIndexCache();

    if ( isVisible() ) {
        updateGridSize();
        repaintScreen();
    }
}


/**
 * Return the file name shown in cell (row,col) if a thumbnail is shown in this cell or null otherwise.
 */
QString ThumbnailView::ThumbnailWidget::fileNameInCell( int row, int col ) const
{
    uint index = row * numCols() + col;
    if ( index >= _imageList.count() )
        return QString::null;
    else
        return _imageList[index];
}

/**
 * Returns the file name shown at viewport position (x,y) if a thumbnail is shown at this position or QString::null otherwise.
 */
QString ThumbnailView::ThumbnailWidget::fileNameAtCoordinate( const QPoint& coordinate, CoordinateSystem system ) const
{
    QPoint contentsPos = coordinate;
    if ( system == ViewportCoordinates )
        contentsPos = viewportToContents( coordinate );

    int col = columnAt( contentsPos.x() );
    int row = rowAt( contentsPos.y() );

    QRect cellRect = const_cast<ThumbnailWidget*>(this)->cellGeometry( row, col );
    QRect iconRect = iconGeometry( row, col );

    // map iconRect from local coordinates within the cell to contents coordinates
    iconRect.moveBy( cellRect.x(), cellRect.y() );

    if ( iconRect.contains( contentsPos ) )
        return fileNameInCell( row, col );
    else
        return QString::null;
}

/**
 * Return the geometry for the icon in the cell (row,col). The returned coordinates are local to the cell.
 */
QRect ThumbnailView::ThumbnailWidget::iconGeometry( int row, int col ) const
{
    QString fileName = fileNameInCell( row, col );
    if ( fileName.isNull() ) // empty cell
        return QRect();

    int size = Settings::SettingsData::instance()->thumbSize() + SPACE;
    QPixmap* pix = QPixmapCache::find( fileName );
    if ( !pix )
        return QRect( SPACE, SPACE, size, size );

    int xoff = 1 + (size - pix->width())/2; // 1 is for the border at the left
    int yoff = (size - pix->height() );
    if ( !Settings::SettingsData::instance()->displayLabels() )
        yoff /= 2; // we wil center the images if we do not show the label, otherwise we will align it to the bottom
    return QRect( xoff, yoff, pix->width(), pix->height() );
}

QRect ThumbnailView::ThumbnailWidget::cellTextGeometry( int row, int col ) const
{
    if ( !Settings::SettingsData::instance()->displayLabels() )
        return QRect();

    QString fileName = fileNameInCell( row, col );
    if ( fileName.isNull() ) // empty cell
        return QRect();


    int h = QFontMetrics(font()).height();
    QRect iconRect = iconGeometry( row, col );
    QRect cellRect = const_cast<ThumbnailWidget*>(this)->cellGeometry( row, col );

    return QRect( 1, cellRect.height() -h -1, cellRect.width()-2, h );
}



void ThumbnailView::ThumbnailWidget::pixmapLoaded( const QString& fileName, const QSize& size, const QSize& fullSize, int,
                                                 const QImage& image, bool loadedOK )
{
    QPixmap* pixmap = new QPixmap( size );
    if ( loadedOK && !image.isNull() )
        pixmap->convertFromImage( image );

    else if ( !loadedOK)
        pixmap->fill( palette().active().dark());

    DB::ImageInfoPtr imageInfo = DB::ImageDB::instance()->info( fileName );

    if ( !loadedOK || !imageInfo->imageOnDisk() ) {
        QPainter p( pixmap );
        p.setBrush( palette().active().base() );
        p.setWindow( 0, 0, 100, 100 );
        QPointArray pts;
        pts.setPoints( 3, 70,-1,  100,-1,  100,30 );
        p.drawConvexPolygon( pts );
    }

    if ( fullSize.isValid() )
        imageInfo->setSize( fullSize );

    QPixmapCache::insert( fileName, pixmap );
    updateCell( fileName );
}

/**
 * Request a repaint of the cell showing filename
 *
 * QGridView::updateCell has some problems when we scroll during a keyboard selection, so thatswhy we role our own one.
 */
void ThumbnailView::ThumbnailWidget::updateCell( const QString& fileName )
{
    if ( fileName.isNull() )
        return;

    _pendingRepaint.insert( fileName );
    _repaintTimer->start( 0, true );
}

void ThumbnailView::ThumbnailWidget::updateCell( int row, int col )
{
    updateCell( fileNameInCell( row, col ) );
}


/**
 * Update the grid size depending on the size of the widget
 */
void ThumbnailView::ThumbnailWidget::updateGridSize()
{
    int thumbnailsPerRow = width() / cellWidth();
    int numRowsPerPage = height() / cellHeight();
    setNumCols( thumbnailsPerRow );
    setNumRows( QMAX( numRowsPerPage, (int) ceil( 1.0 * _imageList.size() / thumbnailsPerRow ) ) );
}

void ThumbnailView::ThumbnailWidget::showEvent( QShowEvent* )
{
    updateGridSize();
}

/**
 * Paint the cell back ground, and the outline
 */
void ThumbnailView::ThumbnailWidget::paintCellBackground( QPainter* p, int row, int col )
{
    QRect rect = cellRect();
    p->fillRect( rect, palette().active().base() );

    p->setPen( palette().active().dark() );
    // left of frame
    if ( col != 0 )
        p->drawLine( rect.left(), rect.top(), rect.left(), rect.bottom() );

    // bottom line
    if ( row != numRows() -1 ) {
        p->drawLine( rect.left(), rect.bottom() -1, rect.right(), rect.bottom()-1 );
        p->setPen( palette().active().light() );
        p->drawLine( rect.left(), rect.bottom() -2, rect.right(), rect.bottom()-2 );
    }
}

void ThumbnailView::ThumbnailWidget::keyPressEvent( QKeyEvent* event )
{
    if ( isMovementKey( event->key() ) )
        keyboardMoveEvent( event );

    if ( event->key() == Key_Return )
        emit showSelection();

    if ( event->key() == Key_Space )
        toggleSelection( _currentItem );

    possibleEmitSelectionChanged();
}

void ThumbnailView::ThumbnailWidget::keyboardMoveEvent( QKeyEvent* event )
{
    static QString startPossition;

    if ( !( event->state()& ShiftButton ) && !( event->state() &  ControlButton ) ) {
        clearSelection();
    }

    // Decide the next keyboard focus cell
    Cell currentPos(0,0);
    if ( !_currentItem.isNull() )
        currentPos = positionForFileName( _currentItem );

    Cell newPos;
    switch (event->key() ) {
    case Key_Left:
        newPos = currentPos;
        newPos.col()--;

        if ( newPos.col() < 0 )
            newPos = Cell( newPos.row()-1, numCols()-1 );
        break;

    case Key_Right:
        newPos = currentPos;
        newPos.col()++;
        if ( newPos.col() == numCols() )
            newPos = Cell( newPos.row()+1, 0 );
        break;

    case Key_Down:
        newPos = Cell( currentPos.row()+1, currentPos.col() );
        break;

    case Key_Up:
        newPos = Cell( currentPos.row()-1, currentPos.col() );
        break;

    case Key_PageDown:
    case Key_PageUp:
    {
        int rows = (event->key() == Key_PageDown) ? 1 : -1;
        if ( event->state() & AltButton )
            rows *= numRows() / 20;
        else
            rows *= numRowsPerPage();

        newPos = Cell( currentPos.row() + rows, currentPos.col() );
        break;
    }
    case Key_Home:
        newPos = Cell( 0, 0 );
        break;

    case Key_End:
        newPos = lastCell();
        break;
    }

    // Check for overruns
    if ( newPos > lastCell() )
        newPos = lastCell();
    if ( newPos < Cell(0,0) )
        newPos = Cell(0,0);

    // Update focus cell, and set selection
    if ( (event->state() & ShiftButton) && !startPossition.isEmpty() )
        selectItems( positionForFileName( startPossition ), newPos );

    if ( ! (event->state() & ControlButton ) ) {
        selectCell( newPos );
        updateCell( currentPos.row(), currentPos.col() );
    }
    _currentItem = fileNameInCell( newPos );
    if ( !( event->state() & ShiftButton ) || startPossition.isEmpty() )
        startPossition = _currentItem;

    // Scroll if necesary
    if ( newPos.row() > lastVisibleRow( ThumbnailWidget::FullyVisible ) )
        setContentsPos( contentsX(), cellGeometry( newPos.row(), newPos.col() ).top() -
                        (numRowsPerPage()-1)*cellHeight()  );

    if  ( newPos.row() < firstVisibleRow( ThumbnailWidget::FullyVisible ) )
        setContentsPos( contentsX(), cellGeometry( newPos.row(), newPos.col() ).top() );
}

/**
 * Update selection to include files from start to end
 */
void ThumbnailView::ThumbnailWidget::selectItems( const Cell& start, const Cell& end )
{
    Set<QString> oldSelection = _selectedFiles;
    _selectedFiles.clear();

    selectAllCellsBetween( start, end, false );

    for( Set<QString>::Iterator it = oldSelection.begin(); it != oldSelection.end(); ++it ) {
        if ( !_selectedFiles.contains( *it ) )
            updateCell( *it );
    }

    for( Set<QString>::Iterator it = _selectedFiles.begin(); it != _selectedFiles.end(); ++it ) {
        if ( !oldSelection.contains( *it ) )
            updateCell( *it );
    }

}
/**
 * Return the number of complete rows per page
 */
int ThumbnailView::ThumbnailWidget::numRowsPerPage() const
{
    return height() / cellHeight();
}

void ThumbnailView::ThumbnailWidget::mousePressEvent( QMouseEvent* event )
{
    if (event->button() & MidButton )
        _mouseHandler = &_gridResizeInteraction;
    else
        _mouseHandler = &_selectionInteraction;

    _mouseHandler->mousePressEvent( event );
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
    if ( !( event->state() & ControlButton ) ) {
        QString fileName = fileNameAtCoordinate( event->pos(), ViewportCoordinates );
        if ( !fileName.isNull() )
            emit showImage( fileName );
    }
}



void ThumbnailView::ThumbnailWidget::emitDateChange( int x, int y )
{
    if ( _isSettingDate )
        return;

    // Unfortunately the contentsMoving signal is emitted *before* the move, so we need to find out what is on the new position ourself.
    QString fileName = fileNameInCell( rowAt(y), columnAt(x) );
    if ( fileName.isNull() )
        return;

    static QDateTime lastDate;
    QDateTime date = DB::ImageDB::instance()->info( fileName )->date().start();
    if ( date != lastDate ) {
        lastDate = date;
        if ( date.date().year() != 1900 )
            emit currentDateChanged( date );
    }
}

/**
 * scroll to the date specified with the parameter date.
 * The boolean includeRanges tells whether we accept range matches or not.
 */
void ThumbnailView::ThumbnailWidget::gotoDate( const DB::ImageDate& date, bool includeRanges )
{
    _isSettingDate = true;
    QString candidate;
    for( QValueVector<QString>::Iterator imageIt = _imageList.begin(); imageIt != _imageList.end(); ++imageIt ) {
        DB::ImageInfoPtr info = DB::ImageDB::instance()->info( *imageIt );

        DB::ImageDate::MatchType match = info->date().isIncludedIn( date );
        if ( match == DB::ImageDate::ExactMatch || ( match == DB::ImageDate::RangeMatch && includeRanges ) ) {
            if ( !candidate.isNull() ) {
                if ( info->date().start() < DB::ImageDB::instance()->info(candidate)->date().start() )
                    candidate = *imageIt;
            }
            else
                candidate = *imageIt;
        }
    }
    if ( !candidate.isNull() ) {
        Cell pos = positionForFileName( candidate );
        QRect contentsRect = cellGeometry( pos.row(), pos.col() );
        setContentsPos( contentsRect.x(), contentsRect.y() );
        _currentItem = candidate;
    }
    _isSettingDate = false;
}

/**
 * return the position (row,col) for the given file name
 */
ThumbnailView::Cell ThumbnailView::ThumbnailWidget::positionForFileName( const QString& fileName ) const
{
    Q_ASSERT( !fileName.isNull() );
    int index = _fileNameMap[fileName];
    if ( index == -1 )
        return Cell( 0, 0 );

    int row = index / numCols();
    int col = index % numCols();
    return Cell( row, col );
}

/**
 * \brief Returns whether the thumbnail for fileName is still needed.
 *
 * If the user scrolls down through the view, a back log of thumbnail
 * request may build up, which will slow down scrolling a lot. Therefore
 * the ImageManger has the capability to check whether a thumbnail
 * request is really needed, when it gets to load the given thumbnail.
 */
bool ThumbnailView::ThumbnailWidget::thumbnailStillNeeded( const QString& fileName ) const
{
    Cell pos = positionForFileName( fileName );
    return pos.row() >= firstVisibleRow( PartlyVisible ) && pos.row() <= lastVisibleRow( PartlyVisible );
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

void ThumbnailView::ThumbnailWidget::selectCell( const Cell& cell )
{
    selectCell( cell.row(), cell.col() );
}

void ThumbnailView::ThumbnailWidget::selectCell( int row, int col, bool repaint )
{
    QString file = fileNameInCell( row, col );
    if ( !file.isNull() ) {
        _selectedFiles.insert( file );
        if ( repaint )
            updateCell( row, col );
    }
}

ThumbnailView::Cell ThumbnailView::ThumbnailWidget::cellAtCoordinate( const QPoint& pos, CoordinateSystem system ) const
{
    QPoint contentsPos = pos;
    if ( system == ViewportCoordinates )
        contentsPos = viewportToContents( pos );

    int col = columnAt( contentsPos.x() );
    int row = rowAt( contentsPos.y() );
    return Cell( row, col );
}

void ThumbnailView::ThumbnailWidget::selectAllCellsBetween( Cell pos1, Cell pos2, bool repaint )
{
    ensureCellsSorted( pos1, pos2 );

    if ( pos1.row() == pos2.row() ) {
        // This is the case where images from only one row is selected.
        for ( int col = pos1.col(); col <= pos2.col(); ++ col )
            selectCell( pos1.row(), col, repaint );
    }
    else {
        // We know we have at least two rows.

        // first row
        for ( int col = pos1.col(); col < numCols(); ++ col )
            selectCell( pos1.row(), col, repaint );

        // rows in between
        for ( int row = pos1.row()+1; row < pos2.row(); ++row )
            for ( int col = 0; col < numCols(); ++ col )
                selectCell( row, col, repaint );

        // last row
        for ( int col = 0; col <= pos2.col(); ++ col )
            selectCell( pos2.row(), col, repaint );
    }
}

void ThumbnailView::ThumbnailWidget::resizeEvent( QResizeEvent* e )
{
    QGridView::resizeEvent( e );
    updateGridSize();
}

void ThumbnailView::ThumbnailWidget::clearSelection()
{
    Set<QString> oldSelection = _selectedFiles;
    _selectedFiles.clear();
    for( Set<QString>::Iterator fileIt = oldSelection.begin(); fileIt != oldSelection.end(); ++fileIt ) {
        updateCell( *fileIt );
    }
}

QString ThumbnailView::ThumbnailWidget::fileNameInCell( const Cell& cell ) const
{
    return fileNameInCell( cell.row(), cell.col() );
}

bool ThumbnailView::ThumbnailWidget::isFocusAtLastCell() const
{
    return positionForFileName(_currentItem) == lastCell();
}

bool ThumbnailView::ThumbnailWidget::isFocusAtFirstCell() const
{
    return positionForFileName(_currentItem) == Cell(0,0);
}

/**
 * Return the coordinates of the last cell with a thumbnail in
 */
ThumbnailView::Cell ThumbnailView::ThumbnailWidget::lastCell() const
{
    return Cell( (_imageList.count()-1) / numCols(),
                 (_imageList.count()-1) % numCols());
}

bool ThumbnailView::ThumbnailWidget::isMovementKey( int key )
{
    return ( key == Key_Up || key == Key_Down || key == Key_Left || key == Key_Right ||
             key == Key_Home || key == Key_End || key == Key_PageUp || key == Key_PageDown );
}

void ThumbnailView::ThumbnailWidget::toggleSelection( const QString& fileName )
{
    if ( _selectedFiles.contains( fileName ) )
        _selectedFiles.remove( fileName );
    else
        _selectedFiles.insert( fileName );

    updateCell( fileName );
}

QStringList ThumbnailView::ThumbnailWidget::selection( bool keepSortOrderOfDatabase ) const
{
    QValueVector<QString> images = _imageList;
    if ( keepSortOrderOfDatabase && _sortDirection == NewestFirst )
        images = reverseVector( images );

    QStringList res;
    for( QValueVector<QString>::ConstIterator it = images.begin(); it != images.end(); ++it ) {
        if ( _selectedFiles.contains( *it ) )
            res.append( *it );
    }
    return res;
}

void ThumbnailView::ThumbnailWidget::possibleEmitSelectionChanged()
{
    static Set<QString> oldSelection;
    if ( oldSelection != _selectedFiles ) {
        oldSelection = _selectedFiles;
        emit selectionChanged();
    }
}

QStringList ThumbnailView::ThumbnailWidget::imageList( Order order ) const
{
    QStringList res = vectorToList( _imageList );

    if ( order == SortedOrder &&  _sortDirection == NewestFirst )
        return reverseList( res );
    else
        return res;
}

void ThumbnailView::ThumbnailWidget::selectAll()
{
    _selectedFiles.clear();
    for( QValueVector<QString>::ConstIterator it = _imageList.begin(); it != _imageList.end(); ++it ) {
        _selectedFiles.insert(*it);
    }
    possibleEmitSelectionChanged();
    repaintScreen();
}

void ThumbnailView::ThumbnailWidget::reload(bool flushCache )
{
    if ( flushCache )
        QPixmapCache::clear();
    _selectedFiles.clear();
    updateCellSize();
    repaintScreen();
}

void ThumbnailView::ThumbnailWidget::repaintScreen()
{
    for ( int row = firstVisibleRow( PartlyVisible ); row <= lastVisibleRow( PartlyVisible ); ++row )
        for ( int col = 0; col < numCols(); ++col )
            QGridView::repaintCell( row, col );
}

QString ThumbnailView::ThumbnailWidget::fileNameUnderCursor() const
{
    return fileNameAtCoordinate( mapFromGlobal( QCursor::pos() ), ViewportCoordinates );
}

QString ThumbnailView::ThumbnailWidget::currentItem() const
{
    return _currentItem;
}

ThumbnailView::ThumbnailWidget* ThumbnailView::ThumbnailWidget::theThumbnailView()
{
    return _instance;
}

void ThumbnailView::ThumbnailWidget::setCurrentItem( const QString& fileName )
{
    Cell cell = positionForFileName( fileName );
    _currentItem = fileName;
    _selectedFiles.clear();
    _selectedFiles.insert( fileName );
    updateCell( fileName );
    ensureCellVisible( cell.row(), cell.col() );
}

void ThumbnailView::ThumbnailWidget::showToolTipsOnImages( bool on )
{
    _toolTip->setActive( on );
}


void ThumbnailView::ThumbnailWidget::contentsDragMoveEvent( QDragMoveEvent* event )
{
    bool accept = event->provides( "text/uri-list" ) && _selectionInteraction.isDragging();
    event->accept( accept );

    if ( !accept )
        return;

    int row = rowAt( event->pos().y() );
    int col = columnAt( event->pos().x() );
    QString fileName = fileNameInCell( row, col );

    removeDropIndications();

    QRect rect = cellGeometry( row, col );
    bool left = ( event->pos().x() - rect.x() < rect.width()/2 );
    if ( left ) {
        _leftDrop = fileName;
        int index = _fileNameMap[fileName] -1;
        if ( index != -1 )
            _rightDrop = _imageList[index];
    }

    else {
        _rightDrop = fileName;
        uint index = _fileNameMap[fileName] +1;
        if ( index != _imageList.count() )
            _leftDrop = _imageList[index];
    }

    updateCell( _leftDrop );
    updateCell( _rightDrop );
}

void ThumbnailView::ThumbnailWidget::contentsDragLeaveEvent( QDragLeaveEvent* )
{
    removeDropIndications();
}

void ThumbnailView::ThumbnailWidget::contentsDropEvent( QDropEvent* )
{
    QTimer::singleShot( 0, this, SLOT( realDropEvent() ) );
}

/**
 * Do the real work for the drop event.
 * We can't bring up the dialog in the contentsDropEvent, as Qt is still in drag and drop mode with a different cursor etc.
 * That's why we use a QTimer to get this call back executed.
 */
void ThumbnailView::ThumbnailWidget::realDropEvent()
{
    QString msg =
        i18n( "<p><b>Really reorder images?</b></p>"
              "<p>By dragging images around in the thumbnail viewer, you actually reorder them. "
              "This is very useful in case you don't know the exact date for the images. On the other hand, "
              "if the images them self has a valid time stamp, you should instead use "
              "<tt>Images -&gt; Sort Selected By Date and Time</tt></p>" );

    if ( KMessageBox::questionYesNo( this, msg, i18n("Reorder Images") , KStdGuiItem::yes(), KStdGuiItem::no(),
                                     QString::fromLatin1( "reorder_images" ) ) == KMessageBox::Yes ) {

        // protect against self drop
        if ( !_selectedFiles.contains( _leftDrop ) && ! _selectedFiles.contains( _rightDrop ) ) {
            QStringList selected = selection();
            if ( _rightDrop.isNull() ) {
                // We dropped onto the first image.
                DB::ImageDB::instance()->reorder( _leftDrop, selected, false );
            }
            else
                DB::ImageDB::instance()->reorder( _rightDrop, selected, true );

            Browser::BrowserWidget::instance()->reload();
        }
    }
    removeDropIndications();
}

void ThumbnailView::ThumbnailWidget::removeDropIndications()
{
    QString left = _leftDrop;
    QString right = _rightDrop;
    _leftDrop = QString::null;
    _rightDrop = QString::null;

    updateCell( left );
    updateCell( right );
}

void ThumbnailView::ThumbnailWidget::ensureCellsSorted( Cell& pos1, Cell& pos2 )
{
    if ( pos2.row() < pos1.row() || ( pos2.row() == pos1.row() && pos2.col() < pos1.col() ) ) {
        Cell tmp = pos1;
        pos1 = pos2;
        pos2 = tmp;
    }
}

void ThumbnailView::ThumbnailWidget::slotRepaint()
{
    if ( (int) _pendingRepaint.count() > numCols() * numRowsPerPage() / 2 )
        repaintScreen();
    else {
        for( Set<QString>::Iterator it = _pendingRepaint.begin(); it != _pendingRepaint.end(); ++it ) {
            Cell cell = positionForFileName( *it );
            QGridView::repaintCell( cell.row(), cell.col() );
        }
    }
    _pendingRepaint.clear();
}

void ThumbnailView::ThumbnailWidget::dimensionChange( int oldNumRows, int /*oldNumCols*/ )
{
    if ( oldNumRows != numRows() )
        repaintScreen();
}

void ThumbnailView::ThumbnailWidget::setSortDirection( SortDirection direction )
{
    if ( direction == _sortDirection )
        return;

    Settings::SettingsData::instance()->setShowNewestFirst( direction == NewestFirst );
    _imageList = reverseVector( _imageList );
    updateIndexCache();
    if ( !_currentItem.isNull() )
        setCurrentItem( _currentItem );
    repaintScreen();

    _sortDirection = direction;
}

QStringList ThumbnailView::ThumbnailWidget::reverseList( const QStringList& list) const
{
    QStringList res;
    for( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
        res.prepend(*it);
    }
    return res;
}

QValueVector<QString> ThumbnailView::ThumbnailWidget::reverseVector( const QValueVector<QString>& vect) const
{
    QValueVector<QString> res;
    int size = vect.count();
    res.resize( size );
    int index = 0;
    for( QValueVector<QString>::ConstIterator it = vect.begin(); it != vect.end(); ++it, ++index ) {
        res[size-1-index] = *it;
    }
    return res;
}


void ThumbnailView::ThumbnailWidget::updateCellSize()
{
    int size = Settings::SettingsData::instance()->thumbSize() + SPACE;
    setCellWidth( size );

    int h = size +2;
    if ( Settings::SettingsData::instance()->displayLabels() )
        h += QFontMetrics( font() ).height() +2;
    setCellHeight( h );
}

void ThumbnailView::ThumbnailWidget::viewportPaintEvent( QPaintEvent* e )
{
    QPainter p( viewport() );
    p.fillRect( numCols() * cellWidth(), 0, width(), height(), palette().active().base() );
    p.fillRect( 0, numRows() * cellHeight(), width(), height(), palette().active().base() );
    QGridView::viewportPaintEvent( e );
}

QStringList ThumbnailView::ThumbnailWidget::vectorToList( const QValueVector<QString>& vect ) const
{
    QStringList res;
    for( QValueVector<QString>::ConstIterator it = vect.begin(); it != vect.end(); ++it ) {
        res.append( *it );
    }
    return res;
}

void ThumbnailView::ThumbnailWidget::updateIndexCache()
{
    _fileNameMap.clear();
    int index = 0;
    for( QValueVector<QString>::ConstIterator it = _imageList.begin(); it != _imageList.end(); ++it,++index ) {
        _fileNameMap.insert( *it, index );
    }
}

void ThumbnailView::ThumbnailWidget::ensureCurrentVisible()
{
    // We need a single shot timer to ensure to be processed way after event processing for keyboard.
    QTimer::singleShot( 0, this, SLOT( ensureCurrentVisiblePart2() ) );
}


void ThumbnailView::ThumbnailWidget::ensureCurrentVisiblePart2()
{
    Cell cur = positionForFileName(_currentItem);
    if ( cur.row() < firstVisibleRow( PartlyVisible ) )
        _currentItem = fileNameInCell( Cell( firstVisibleRow( FullyVisible ), cur.col() ) );
    else if ( cur.row() > lastVisibleRow( PartlyVisible ) )
        _currentItem = fileNameInCell( Cell( lastVisibleRow( FullyVisible ), cur.col() ) );
}

