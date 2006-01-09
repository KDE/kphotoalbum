#include "ThumbnailView.h"
#include <qpixmapcache.h>
#include <qpainter.h>
#include "math.h"
#include "options.h"
#include "ThumbnailRequest.h"
#include "imagemanager.h"
#include "imageinfoptr.h"
#include "imagedb.h"
#include "util.h"
#include <qcursor.h>
#include <qapplication.h>
#include "ThumbnailToolTip.h"
#include "browser.h"
#include <klocale.h>
#include <kmessagebox.h>

// PENDING(blackie) TODO:
// - show file names

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

ThumbnailView::ThumbnailView* ThumbnailView::ThumbnailView::_instance = 0;
ThumbnailView::ThumbnailView::ThumbnailView( QWidget* parent, const char* name )
    :QGridView( parent, name ),
     _gridResizeInteraction( this ),
     _selectionInteraction( this ),
     _mouseTrackingHandler( this ),
     _mouseHandler( &_mouseTrackingHandler )
{
    _instance = this;
    int size = Options::instance()->thumbSize() + SPACE;
    setCellWidth( size );
    setCellHeight( size +2 );
    setFocusPolicy( WheelFocus );

    // It beats me why I need to set mouse tracking on both, but without it doesn't work.
    viewport()->setMouseTracking( true );
    setMouseTracking( true );

    connect( this, SIGNAL( contentsMoving( int, int ) ), this, SLOT( emitDateChange( int, int ) ) );

    _toolTip = new ThumbnailToolTip( this );

    viewport()->setAcceptDrops( true );
}


void ThumbnailView::ThumbnailView::paintCell( QPainter * p, int row, int col )
{
    QPixmap doubleBuffer( cellRect().size() );
    QPainter painter( &doubleBuffer );
    paintCellBackground( &painter, row, col );

    QString fileName = fileNameInCell( row, col );
    if ( !fileName.isNull() && !_mouseHandler->isResizingGrid() ) {
        QPixmap* pix = pixmapCache().find( fileNameInCell( row, col ) );
        if ( pix ) {
            QRect rect = iconGeometry( row, col );
            Q_ASSERT( !rect.isNull() );
            painter.drawPixmap( rect, *pix );
            if ( _selectedFiles.contains( fileName ) )
                painter.fillRect( rect, QBrush( palette().active().highlight(), Dense4Pattern ) );

            rect = QRect( 0, 0, cellWidth(), cellHeight() );
            if ( _leftDrop == fileName )
                painter.fillRect( rect.left(), rect.top(), 3, rect.height(), QBrush( red ) );
            else if ( _rightDrop == fileName )
                painter.fillRect( rect.right() -2, rect.top(), 3, rect.height(), QBrush( red ) );

        }
        else {
            int size = Options::instance()->thumbSize();
            int angle = ImageDB::instance()->info( fileName )->angle();
            ThumbnailRequest* request = new ThumbnailRequest( fileName, QSize( size, size ), angle, this );
            request->setCache();
            ImageManager::instance()->load( request );
        }
    }

    painter.end();
    p->drawPixmap( cellRect(), doubleBuffer );
}


void ThumbnailView::ThumbnailView::setImageList( const QStringList& list )
{
    _imageList = list;
    if ( isVisible() ) {
        updateGridSize();
        update();
    }
}


/**
 * Calculate the about of thumbnails per row.
 */
int ThumbnailView::ThumbnailView::thumbnailsPerRow() const
{
    return width() / cellWidth();
}


/**
 * Return the file name shown in cell (row,col) if a thumbnail is shown in this cell or null otherwise.
 */
QString ThumbnailView::ThumbnailView::fileNameInCell( int row, int col ) const
{
    uint index = row * thumbnailsPerRow() + col;
    if ( index >= _imageList.count() )
        return QString::null;
    else
        return _imageList[index];
}

/**
 * Returns the file name shown at viewport position (x,y) if a thumbnail is shown at this position or QString::null otherwise.
 */
QString ThumbnailView::ThumbnailView::fileNameAtCoordinate( const QPoint& coordinate, CoordinateSystem system ) const
{
    QPoint contentsPos = coordinate;
    if ( system == ViewportCoordinates )
        contentsPos = viewportToContents( coordinate );

    int col = columnAt( contentsPos.x() );
    int row = rowAt( contentsPos.y() );

    QRect cellRect = const_cast<ThumbnailView*>(this)->cellGeometry( row, col );
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
QRect ThumbnailView::ThumbnailView::iconGeometry( int row, int col ) const
{
    QString fileName = fileNameInCell( row, col );
    if ( fileName.isNull() ) // empty cell
        return QRect();

    QPixmap* pix = const_cast<ThumbnailView*>(this)->pixmapCache().find( fileName );
    if ( !pix )
        return QRect();

    int size = Options::instance()->thumbSize() + SPACE;
    int xoff = 1 + (size - pix->width())/2; // 1 is for the border at the left
    int yoff = 1 + (size - pix->height() )/2;
    return QRect( xoff, yoff, pix->width(), pix->height() );
}


/**
 * Return a pixmap cache to use for caching the thumbnails.
 */
QPixmapCache& ThumbnailView::ThumbnailView::pixmapCache()
{
    static QPixmapCache cache;
    static int lastSize = -1;
    cache.setCacheLimit( 4* 1024 ); // PENDING(blackie) make this size customizable
    int currentThumbSize = Options::instance()->thumbSize();
    if (lastSize != currentThumbSize) {
        cache.clear();
        lastSize = currentThumbSize;
    }
    return cache;
}

void ThumbnailView::ThumbnailView::pixmapLoaded( const QString& fileName, const QSize& size, const QSize& fullSize, int, const QImage& image,
                                  bool loadedOK )
{
    QPixmap* pixmap = new QPixmap( size );
    if ( loadedOK && !image.isNull() )
        pixmap->convertFromImage( image );

    else if ( !loadedOK)
        pixmap->fill(Qt::gray);

    ImageInfoPtr imageInfo = ImageDB::instance()->info( fileName );

    if ( !loadedOK || !imageInfo->imageOnDisk() ) {
        QPainter p( pixmap );
        p.setBrush( white );
        p.setWindow( 0, 0, 100, 100 );
        QPointArray pts;
        pts.setPoints( 3, 70,-1,  100,-1,  100,30 );
        p.drawConvexPolygon( pts );
    }

    if ( fullSize.isValid() )
        imageInfo->setSize( fullSize );

    pixmapCache().insert( fileName, pixmap );
    repaintCell( fileName );
}

/**
 * Request a repaint of the cell showing filename
 */
void ThumbnailView::ThumbnailView::repaintCell( const QString& fileName )
{
    // This ought really be updateCell, but I get errors where cells are
    // not repainted though requested, during scrolling.
    Cell pos = positionForFileName( fileName );
    repaintCell( pos.row(), pos.col() );
}

/**
 * Update the grid size depending on the size of the widget
 */
void ThumbnailView::ThumbnailView::updateGridSize()
{
    setNumCols( thumbnailsPerRow() );
    setNumRows( QMAX( numRowsPerPage(), (int) ceil( 1.0 * _imageList.size() / thumbnailsPerRow() ) ) );
}

void ThumbnailView::ThumbnailView::showEvent( QShowEvent* )
{
    updateGridSize();
}

/**
 * Paint the cell back ground, and the outline
 */
void ThumbnailView::ThumbnailView::paintCellBackground( QPainter* p, int row, int col )
{
    QRect rect = cellRect();
    p->fillRect( rect, black );
    p->fillRect( QRect( QPoint( rect.left(), rect.top()+1), QPoint( rect.right(), rect.bottom()-1 ) ), white );
    p->setPen( gray );

    // left of frame
    if ( col != 0 )
        p->drawLine( rect.left(), rect.top()+1, rect.left(), rect.bottom()-1 );

    // bottom line
    if ( row != numRows() -1 ) {
        // Draw a grey line to give an illusion of a raised box.
        p->drawLine( rect.left(), rect.bottom() -1, rect.right(), rect.bottom()-1 );
    }
    else {
        // We don't want a seperating line at the bottom, but we've already drawn the black one
        // during the fillRect above, so lets undraw it now.
        p->setPen( white );
        p->drawLine( rect.left(), rect.bottom(), rect.right(), rect.bottom() );
    }

    if ( row == 0 ) {
        // undraw the topmost line of row zero
        p->setPen( white );
        p->drawLine( rect.left(), rect.top(), rect.right(), rect.top() );
    }
}

void ThumbnailView::ThumbnailView::keyPressEvent( QKeyEvent* event )
{
    if ( isMovementKey( event->key() ) )
        keyboardMoveEvent( event );

    if ( event->key() == Key_Return )
        emit showImage( _currentItem );

    if ( event->key() == Key_Space )
        toggleSelection( _currentItem );

    possibleEmitSelectionChanged();
}

void ThumbnailView::ThumbnailView::keyboardMoveEvent( QKeyEvent* event )
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
            newPos = Cell( newPos.row()-1, thumbnailsPerRow()-1 );
        break;

    case Key_Right:
        newPos = currentPos;
        newPos.col()++;
        if ( newPos.col() == thumbnailsPerRow() )
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
    if ( event->state() & ShiftButton )
        selectItems( positionForFileName( startPossition ), newPos );

    else if ( ! (event->state() & ControlButton ) ) {
        selectCell( newPos );
        repaintCell( currentPos.row(), currentPos.col() );
    }
    _currentItem = fileNameInCell( newPos );
    if ( !( event->state() & ShiftButton ) )
        startPossition = _currentItem;

    // Scroll if necesary
    if ( newPos.row() > lastVisibleRow( ThumbnailView::FullyVisible ) )
        setContentsPos( contentsX(), cellGeometry( newPos.row(), newPos.col() ).top() -
                        (numRowsPerPage()-1)*cellHeight()  );

    if  ( newPos.row() < firstVisibleRow( ThumbnailView::FullyVisible ) )
        setContentsPos( contentsX(), cellGeometry( newPos.row(), newPos.col() ).top() );
}

/**
 * Update selection to include files from start to end
 */
void ThumbnailView::ThumbnailView::selectItems( const Cell& start, const Cell& end )
{
    Set<QString> oldSelection = _selectedFiles;
    _selectedFiles.clear();

    selectAllCellsBetween( start, end, false );

    for( Set<QString>::Iterator it = oldSelection.begin(); it != oldSelection.end(); ++it ) {
        if ( !_selectedFiles.contains( *it ) )
            repaintCell( *it );
    }

    for( Set<QString>::Iterator it = _selectedFiles.begin(); it != _selectedFiles.end(); ++it ) {
        if ( !oldSelection.contains( *it ) )
            repaintCell( *it );
    }

}
/**
 * Return the number of complete rows per page
 */
int ThumbnailView::ThumbnailView::numRowsPerPage() const
{
    return height() / cellHeight();
}

void ThumbnailView::ThumbnailView::mousePressEvent( QMouseEvent* event )
{
    if ( event->button() == RightButton )
        return;
    else if (event->button() & MidButton )
        _mouseHandler = &_gridResizeInteraction;
    else
        _mouseHandler = &_selectionInteraction;

    _mouseHandler->mousePressEvent( event );
}

void ThumbnailView::ThumbnailView::mouseMoveEvent( QMouseEvent* event )
{
    _mouseHandler->mouseMoveEvent( event );
}

void ThumbnailView::ThumbnailView::mouseReleaseEvent( QMouseEvent* event )
{
    _mouseHandler->mouseReleaseEvent( event );
    _mouseHandler = &_mouseTrackingHandler;
}

void ThumbnailView::ThumbnailView::mouseDoubleClickEvent( QMouseEvent * event )
{
    QString fileName = fileNameAtCoordinate( event->pos(), ViewportCoordinates );
    if ( !fileName.isNull() )
        emit showImage( fileName );
}


void ThumbnailView::ThumbnailView::emitDateChange( int x, int y )
{
    if ( _isSettingDate )
        return;

    // Unfortunately the contentsMoving signal is emitted *before* the move, so we need to find out what is on the new position ourself.
    QString fileName = fileNameInCell( rowAt(y), columnAt(x) );
    if ( fileName.isNull() )
        return;

    static QDateTime lastDate;
    QDateTime date = ImageDB::instance()->info( fileName )->date().start();
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
void ThumbnailView::ThumbnailView::gotoDate( const ImageDateRange& date, bool includeRanges )
{
    _isSettingDate = true;
    QString candidate;
    for( QStringList::Iterator imageIt = _imageList.begin(); imageIt != _imageList.end(); ++imageIt ) {
        ImageInfoPtr info = ImageDB::instance()->info( *imageIt );

        ImageDateRange::MatchType match = info->dateRange().isIncludedIn( date );
        if ( match == ImageDateRange::ExactMatch || ( match == ImageDateRange::RangeMatch && includeRanges ) ) {
            if ( !candidate.isNull() ) {
                if ( info->date().start() < ImageDB::instance()->info(candidate)->date().start() )
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
ThumbnailView::Cell ThumbnailView::ThumbnailView::positionForFileName( const QString& fileName ) const
{
    Q_ASSERT( !fileName.isNull() );
    int index = _imageList.findIndex( fileName );
    Q_ASSERT ( index != -1 );
#ifdef TEMPORARILY_REMOVED // I'll bet you I'll get in trouble here again
    if ( index == -1 )
        qDebug("Ups");
#endif

    int row = index / thumbnailsPerRow();
    int col = index % thumbnailsPerRow();
    return Cell( row, col );
}

/**
 * \brief Returns whether the thumbnail for fileName is still needed.
 *
 * If the user scrolls down through the view, a back log of thumbnail
 * request may build up, which will slow down scrolling a lot. Therefore
 * the \ref ImageManger has the capability to check whether a thumbnail
 * request is really needed, when it gets to load the given thumbnail.
 */
bool ThumbnailView::ThumbnailView::thumbnailStillNeeded( const QString& fileName ) const
{
    Cell pos = positionForFileName( fileName );
    return pos.row() >= firstVisibleRow( PartlyVisible ) && pos.row() <= lastVisibleRow( PartlyVisible );
}

/*
 * Returns the first row that is at least partly visible.
 */
int ThumbnailView::ThumbnailView::firstVisibleRow( VisibleState state ) const
{
    int firstRow = rowAt( contentsY() );
    if ( state == FullyVisible && rowAt( contentsY() + cellHeight()-1 ) != firstRow )
        firstRow += 1;

    return firstRow;
}

int ThumbnailView::ThumbnailView::lastVisibleRow( VisibleState state ) const
{
    int lastRow = rowAt( contentsY() + visibleHeight() );
    if ( state == FullyVisible && rowAt( contentsY() + visibleHeight() - cellHeight() -1 ) != lastRow )
        lastRow -= 1;
    return lastRow;
}

void ThumbnailView::ThumbnailView::selectCell( const Cell& cell )
{
    selectCell( cell.row(), cell.col() );
}

void ThumbnailView::ThumbnailView::selectCell( int row, int col, bool repaint )
{
    QString file = fileNameInCell( row, col );
    if ( !file.isNull() ) {
        _selectedFiles.insert( file );
        if ( repaint )
            repaintCell( row, col );
    }
}

ThumbnailView::Cell ThumbnailView::ThumbnailView::cellAtCoordinate( const QPoint& pos, CoordinateSystem system ) const
{
    QPoint contentsPos = pos;
    if ( system == ViewportCoordinates )
        contentsPos = viewportToContents( pos );

    int col = columnAt( contentsPos.x() );
    int row = rowAt( contentsPos.y() );
    return Cell( row, col );
}

void ThumbnailView::ThumbnailView::selectAllCellsBetween( Cell pos1, Cell pos2, bool repaint )
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
        for ( int col = pos1.col(); col < thumbnailsPerRow(); ++ col )
            selectCell( pos1.row(), col, repaint );

        // rows in between
        for ( int row = pos1.row()+1; row < pos2.row(); ++row )
            for ( int col = 0; col < thumbnailsPerRow(); ++ col )
                selectCell( row, col, repaint );

        // last row
        for ( int col = 0; col <= pos2.col(); ++ col )
            selectCell( pos2.row(), col, repaint );
    }
}

void ThumbnailView::ThumbnailView::resizeEvent( QResizeEvent* e )
{
    QGridView::resizeEvent( e );
    updateGridSize();
}

void ThumbnailView::ThumbnailView::clearSelection()
{
    Set<QString> oldSelection = _selectedFiles;
    _selectedFiles.clear();
    for( Set<QString>::Iterator fileIt = oldSelection.begin(); fileIt != oldSelection.end(); ++fileIt ) {
        repaintCell( *fileIt );
    }
}

QString ThumbnailView::ThumbnailView::fileNameInCell( const Cell& cell ) const
{
    return fileNameInCell( cell.row(), cell.col() );
}

bool ThumbnailView::ThumbnailView::isFocusAtLastCell() const
{
    return positionForFileName(_currentItem) == lastCell();
}

bool ThumbnailView::ThumbnailView::isFocusAtFirstCell() const
{
    return positionForFileName(_currentItem) == Cell(0,0);
}

/**
 * Return the coordinates of the last cell with a thumbnail in
 */
ThumbnailView::Cell ThumbnailView::ThumbnailView::lastCell() const
{
    return Cell( (_imageList.count()-1) / thumbnailsPerRow(),
                 (_imageList.count()-1) % thumbnailsPerRow());
}

bool ThumbnailView::ThumbnailView::isMovementKey( int key )
{
    return ( key == Key_Up || key == Key_Down || key == Key_Left || key == Key_Right ||
             key == Key_Home || key == Key_End || key == Key_PageUp || key == Key_PageDown );
}

void ThumbnailView::ThumbnailView::toggleSelection( const QString& fileName )
{
    if ( _selectedFiles.contains( fileName ) )
        _selectedFiles.remove( fileName );
    else
        _selectedFiles.insert( fileName );

    repaintCell( fileName );
}

QStringList ThumbnailView::ThumbnailView::selection() const
{
    QStringList res;
    for( QStringList::ConstIterator it = _imageList.begin(); it != _imageList.end(); ++it ) {
        if ( _selectedFiles.contains( *it ) )
            res.append( *it );
    }
    return res;
}

void ThumbnailView::ThumbnailView::possibleEmitSelectionChanged()
{
    static Set<QString> oldSelection;
    if ( oldSelection != _selectedFiles ) {
        oldSelection = _selectedFiles;
        emit selectionChanged();
    }
}

QStringList ThumbnailView::ThumbnailView::imageList() const
{
    return _imageList;
}

void ThumbnailView::ThumbnailView::selectAll()
{
    _selectedFiles.clear();
    for( QStringList::ConstIterator it = _imageList.begin(); it != _imageList.end(); ++it ) {
        _selectedFiles.insert(*it);
    }
    repaintScreen();
}

void ThumbnailView::ThumbnailView::repaintCell( int row, int col )
{
    QGridView::repaintCell( row, col );
}

void ThumbnailView::ThumbnailView::reload()
{
    pixmapCache().clear();
    _selectedFiles.clear();
    repaintScreen();
}

void ThumbnailView::ThumbnailView::repaintScreen()
{
    for ( int row = firstVisibleRow( PartlyVisible ); row <= lastVisibleRow( PartlyVisible ); ++row )
        for ( int col = 0; col < thumbnailsPerRow(); ++col )
            repaintCell( row, col );
}

QString ThumbnailView::ThumbnailView::fileNameUnderCursor() const
{
    return fileNameAtCoordinate( mapFromGlobal( QCursor::pos() ), ViewportCoordinates );
}

QString ThumbnailView::ThumbnailView::currentItem() const
{
    return _currentItem;
}

ThumbnailView::ThumbnailView* ThumbnailView::ThumbnailView::theThumbnailView()
{
    return _instance;
}

void ThumbnailView::ThumbnailView::setCurrentItem( const QString& fileName )
{
    Cell cell = positionForFileName( fileName );
    _currentItem = fileName;
    _selectedFiles.clear();
    _selectedFiles.insert( fileName );
    repaintCell( fileName );
    ensureCellVisible( cell.row(), cell.col() );
}

void ThumbnailView::ThumbnailView::showToolTipsOnImages( bool on )
{
    _toolTip->setActive( on );
}


void ThumbnailView::ThumbnailView::contentsDragMoveEvent( QDragMoveEvent* event )
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
        int index = _imageList.findIndex( fileName ) -1;
        if ( index != -1 )
            _rightDrop = _imageList[index];
    }

    else {
        _rightDrop = fileName;
        uint index = _imageList.findIndex( fileName ) +1;
        if ( index != _imageList.count() )
            _leftDrop = _imageList[index];
    }

    repaintCell( _leftDrop );
    repaintCell( _rightDrop );
}

void ThumbnailView::ThumbnailView::contentsDragLeaveEvent( QDragLeaveEvent* )
{
    removeDropIndications();
}

void ThumbnailView::ThumbnailView::contentsDropEvent( QDropEvent* )
{
    QTimer::singleShot( 0, this, SLOT( realDropEvent() ) );
}

/**
 * Do the real work for the drop event.
 * We can't bring up the dialog in the contentsDropEvent, as Qt is still in drag and drop mode with a different cursor etc.
 * That's why we use a QTimer to get this call back executed.
 */
void ThumbnailView::ThumbnailView::realDropEvent()
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
                ImageDB::instance()->reorder( _leftDrop, selected, false );
            }
            else
                ImageDB::instance()->reorder( _rightDrop, selected, true );

            Browser::instance()->reload();
        }
    }
    removeDropIndications();
}

void ThumbnailView::ThumbnailView::removeDropIndications()
{
    QString left = _leftDrop;
    QString right = _rightDrop;
    _leftDrop = QString::null;
    _rightDrop = QString::null;

    repaintCell( left );
    repaintCell( right );
}

void ThumbnailView::ThumbnailView::ensureCellsSorted( Cell& pos1, Cell& pos2 )
{
    if ( pos2.row() < pos1.row() || ( pos2.row() == pos1.row() && pos2.col() < pos1.col() ) ) {
        Cell tmp = pos1;
        pos1 = pos2;
        pos2 = tmp;
    }
}

