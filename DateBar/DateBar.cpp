/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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

#include "DateBar.h"
#include <qdatetime.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qwmatrix.h>
#include <qfontmetrics.h>
#include "ViewHandler.h"
#include <qtoolbutton.h>
#include <qpopupmenu.h>
#include <qaction.h>
#include <qtimer.h>
#include <qcursor.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <math.h>
#include <klocale.h>
#include "options.h"
#include <qapplication.h>

const int borderAboveHistogram = 4;
const int borderArroundWidget = 0;
const int buttonWidth = 22;
const int arrowLength = 20;

/**
 * \namespace DateBar
 * \brief The date bar at the bottom of the main window
 */

/**
 * \class DateBar::DateBar
 * \brief This class represents the date bar at the bottom of the main window.
 *
 * The mouse interaction is handled by the classes which inherits \ref DateBar::MouseHandler, while the logic for
 * deciding the length (in minutes, hours, days, etc) are handled by subclasses of \ref DateBar::ViewHandler.
 */

DateBar::DateBar::DateBar( QWidget* parent, const char* name )
    :QWidget( parent, name ), _currentHandler( &_yearViewHandler ),_tp(YearView), _currentMouseHandler(0),
     _currentDate( QDateTime::currentDateTime() ),_includeFuzzyCounts( true ), _contextMenu(0),
     _showResolutionIndicator( true )
{
    setBackgroundMode( NoBackground );
    setMouseTracking( true );
    setFocusPolicy( StrongFocus );

    _barWidth = Options::instance()->histogramSize().width();
    _barHeight = Options::instance()->histogramSize().height();
    _rightArrow = new QToolButton( RightArrow, this );
    connect( _rightArrow, SIGNAL( clicked() ), this, SLOT( scrollRight() ) );

    _leftArrow = new QToolButton( LeftArrow, this );
    connect( _leftArrow, SIGNAL( clicked() ), this, SLOT( scrollLeft() ) );

    _zoomIn = new QToolButton( this );
    _zoomIn->setIconSet( KGlobal::iconLoader()->loadIconSet( QString::fromLatin1( "viewmag+" ), KIcon::Toolbar, 16 ) );
    connect( _zoomIn, SIGNAL( clicked() ), this, SLOT( zoomIn() ) );
    connect( this, SIGNAL(canZoomIn(bool)), _zoomIn, SLOT( setEnabled( bool ) ) );

    _zoomOut = new QToolButton( this );
    _zoomOut->setIconSet(  KGlobal::iconLoader()->loadIconSet( QString::fromLatin1( "viewmag-" ), KIcon::Toolbar, 16 ) );
    connect( _zoomOut, SIGNAL( clicked() ), this, SLOT( zoomOut() ) );
    connect( this, SIGNAL(canZoomOut(bool)), _zoomOut, SLOT( setEnabled( bool ) ) );

    _cancelSelection = new QToolButton( this );
    _cancelSelection->setIconSet( KGlobal::iconLoader()->loadIconSet( QString::fromLatin1( "cancel" ), KIcon::Toolbar, 16 ) );
    connect( _cancelSelection, SIGNAL( clicked() ), this, SLOT( clearSelection() ) );
    _cancelSelection->setEnabled( false );

    placeAndSizeButtons();

    _focusItemDragHandler = new FocusItemDragHandler( this );
    _barDragHandler = new BarDragHandler( this );
    _selectionHandler = new SelectionHandler( this );

}

QSize DateBar::DateBar::sizeHint() const
{
    int height = QMAX( dateAreaGeometry().bottom() + borderArroundWidget,
                       _barHeight+ buttonWidth + 2* borderArroundWidget + 7 );
    return QSize( 800, height );
}

QSize DateBar::DateBar::minimumSizeHint() const
{
     int height = QMAX( dateAreaGeometry().bottom() + borderArroundWidget,
                        _barHeight + buttonWidth + 2* borderArroundWidget + 7 );
     return QSize( 200, height );
}

void DateBar::DateBar::paintEvent( QPaintEvent* /*event*/ )
{
    bitBlt( this, 0,0, &_buffer );
}

void DateBar::DateBar::redraw()
{
    if ( _buffer.isNull() )
        return;

    if (_dates == 0 )
        return;

    QPainter p( &_buffer );
    p.setFont( font() );
    QColorGroup grp = palette().active();

    // Fill with background pixels
    p.save();
    p.setPen( NoPen );
    p.setBrush( grp.background() );
    p.drawRect( rect() );

    // Draw the area with histograms
    QRect barArea = barAreaGeometry();

    p.setPen( grp.dark() );
    p.setBrush( grp.base() );
    p.drawRect( barArea );
    p.restore();

    _currentHandler->init( dateForUnit( -_currentUnit, _currentDate ) );

    int right;
    drawResolutionIndicator( p, &right );
    QRect rect = dateAreaGeometry();
    rect.setRight( right );
    rect.setLeft( rect.left() + buttonWidth + 2 );

    drawTickMarks( p, rect );
    drawHistograms( p );
    drawFocusRectagle( p );
    updateArrowState();
    repaint();
}

void DateBar::DateBar::resizeEvent( QResizeEvent* event )
{
    placeAndSizeButtons();
    _buffer.resize( event->size() );
    _currentUnit = numberOfUnits()/2;
    redraw();
}

void DateBar::DateBar::drawTickMarks( QPainter& p, const QRect& textRect )
{
    QRect rect = tickMarkGeometry();
    p.save();
    p.setPen( QPen( palette().active().text(), 1 ) );

    QFont f( font() );
    QFontMetrics fm(f);
    int fontHeight = fm.height();
    int unit = 0;
    QRect clip = rect;
    clip.setHeight( rect.height() + 2 + fontHeight );
    clip.setLeft( clip.left() + 2 );
    clip.setRight( clip.right() -2 );
    p.setClipRect( clip );

    for ( int x = rect.x(); x < rect.right(); x+=_barWidth, unit += 1 ) {
        // draw selection indication
        p.save();
        p.setPen( NoPen );
        p.setBrush( palette().active().highlight() );
        QDateTime date = dateForUnit( unit );
        if ( isUnitSelected( unit ) )
            p.drawRect( QRect( x, rect.top(), _barWidth, rect.height() ) );
        p.restore();

        // draw tickmarks
        int h = rect.height();
        if ( _currentHandler->isMajorUnit( unit ) ) {
            QString text = _currentHandler->text( unit );
            int w = fm.width( text );
            p.setFont( f );
            if ( textRect.right() >  x + w/2 && textRect.left() < x - w/2)
                p.drawText( x - w/2, textRect.top(), w, fontHeight, Qt::SingleLine, text );
        }
        else if ( _currentHandler->isMidUnit( unit ) )
            h = (int) ( 2.0/3*rect.height());
        else
            h = (int) ( 1.0/3*rect.height());

        p.drawLine( x, rect.top(), x, rect.top() + h );
    }

    p.restore();
}

void DateBar::DateBar::setViewType( ViewType tp )
{
    switch ( tp ) {
    case DecadeView: _currentHandler = &_decadeViewHandler; break;
    case YearView: _currentHandler = &_yearViewHandler; break;
    case MonthView: _currentHandler = &_monthViewHandler; break;
    case WeekView: _currentHandler = &_weekViewHandler; break;
    case DayView: _currentHandler = &_dayViewHandler; break;
    case HourView: _currentHandler = &_hourViewHandler; break;
    }
    redraw();
    _tp = tp;
}

void DateBar::DateBar::setDate( const QDateTime& date )
{
    _currentDate = date;
    if ( hasSelection() ) {
        if ( currentSelection().start() > _currentDate )
            _currentDate = currentSelection().start();
        if ( currentSelection().end() < _currentDate )
            _currentDate = currentSelection().end();
    }

    if ( unitForDate( _currentDate ) != -1 )
        _currentUnit = unitForDate( _currentDate );

    redraw();
}

void DateBar::DateBar::setImageDateCollection( const KSharedPtr<ImageDateCollection>& dates )
{
    _dates = dates;
    redraw();
}

void DateBar::DateBar::drawHistograms( QPainter& p)
{
    QRect rect = barAreaGeometry();
    p.save();
    p.setClipping( true );
    p.setClipRect( rect );
    p.setPen( NoPen );

    int unit = 0;
    int max = 0;
    for ( int x = rect.x(); x + _barWidth < rect.right(); x+=_barWidth, unit += 1 ) {
        ImageCount count = _dates->count( ImageDate( dateForUnit(unit), dateForUnit(unit+1).addSecs(-1) ) );
        int cnt = count._exact;
        if ( _includeFuzzyCounts )
            cnt += count._rangeMatch;
        max = QMAX( max, cnt  );
    }

    unit = 0;
    for ( int x = rect.x(); x  + _barWidth < rect.right(); x+=_barWidth, unit += 1 ) {
        ImageCount count = _dates->count( ImageDate( dateForUnit(unit), dateForUnit(unit+1).addSecs(-1) ) );
        int exact = 0;
        if ( max != 0 )
            exact = (int) ((double) (rect.height()-2) * count._exact / max );
        int range = 0;
        if ( _includeFuzzyCounts && max != 0 )
            range = (int) ((double) (rect.height()-2) * count._rangeMatch / max );

        BrushStyle style = SolidPattern;
        if ( !isUnitSelected( unit ) && hasSelection() )
            style= Dense5Pattern;

        p.setBrush( QBrush( yellow, style ) );
        p.drawRect( x+1, rect.bottom()-range, _barWidth-2, range );
        p.setBrush( QBrush( green, style ) );
        p.drawRect( x+1, rect.bottom()-range-exact, _barWidth-2, exact );

        // calculate the font size for the largest number.
        QFont f = font();
        bool found = false;
        for ( int i = f.pointSize(); i >= 6; i-=2 ) {
            f.setPointSize( i );
            int w = QFontMetrics(f).width( QString::number( max ) );
            if ( w < rect.height() - 6 ) {
                p.setFont(f);
                found = true;
                break;
            }
        }

        // draw the numbers
        int tot = count._exact;
        if ( _includeFuzzyCounts )
            tot += count._rangeMatch;
        p.save();
        p.translate( x+_barWidth-3, rect.bottom()-2 );
        p.rotate( -90 );
        int w = QFontMetrics(f).width( QString::number( tot ) );
        if ( w < exact+range-2 ) {
            p.drawText( 0,0, QString::number( tot ) );
        }
        p.restore();
    }

    p.restore();
}

void DateBar::DateBar::scrollLeft()
{
    scroll( -1 );
}

void DateBar::DateBar::scrollRight()
{
    scroll( 1 );
}

void DateBar::DateBar::scroll( int units )
{
    _currentDate = dateForUnit( units, _currentDate );
    redraw();
    emit dateSelected( currentDateRange(), includeFuzzyCounts() );
}

void DateBar::DateBar::drawFocusRectagle( QPainter& p)
{
    QRect rect = barAreaGeometry();
    p.save();
    int x = rect.left() + _currentUnit*_barWidth;
    QRect inner( QPoint(x-1, borderAboveHistogram),
                 QPoint( x + _barWidth, borderAboveHistogram + _barHeight - 1 ) );

    p.setPen( QPen( palette().active().dark(), 1 ) );

    // Inner rect
    p.drawRect( inner );
    QRect outer = inner;
    outer.addCoords( -2, -2, 2, 2 );

    // Outer rect
    QRegion region = outer;
    region -= inner;
    p.setClipping( true );
    p.setClipRegion( region );

    QColor col = gray;
    if ( !hasFocus() )
        col = white;

    p.setBrush( col );
    p.setPen( col );
    p.drawRect( outer );

    // Shadow below
    QRect shadow = outer;
    shadow.addCoords( -1,-1, 1, 1 );
    region = shadow;
    region -= outer;
    p.setPen( palette().active().shadow() );
    p.setClipRegion( region );
    p.drawRect( shadow );

    // Light above
    QRect hide = shadow;
    hide.moveBy( 1, 1 );
    region = shadow;
    region -= hide;
    p.setPen( palette().active().light() );
    p.setClipRegion( region );
    p.drawRect( shadow );

    p.restore();
}

void DateBar::DateBar::zoomIn()
{
    if ( _tp == HourView )
        return;
    zoom(+1);
}

void DateBar::DateBar::zoomOut()
{
    if ( _tp == DecadeView )
        return;
    zoom(-1);
}

void DateBar::DateBar::zoom( int factor )
{
    ViewType tp = (ViewType) (_tp+factor);
    setViewType( tp );
    emit canZoomIn( tp != HourView );
    emit canZoomOut( tp != DecadeView );
}

void DateBar::DateBar::mousePressEvent( QMouseEvent* event )
{
    if ( (event->button() & LeftButton) == 0 ||  event->x() > barAreaGeometry().right() || event->x() < barAreaGeometry().left() )
        return;

    if ( event->state() & ControlButton ) {
        _currentMouseHandler = _barDragHandler;
    }
    else {
        bool onBar = event->y() > barAreaGeometry().bottom();
        if ( onBar )
            _currentMouseHandler = _selectionHandler;
        else {
            _currentMouseHandler= _focusItemDragHandler;
        }
    }
    _currentMouseHandler->mousePressEvent( event->x() );
    _cancelSelection->setEnabled( hasSelection() );
    emit dateSelected( currentDateRange(), includeFuzzyCounts() );
    showStatusBarTip( event->pos() );
    redraw();
}

void DateBar::DateBar::mouseReleaseEvent( QMouseEvent* )
{
    _currentMouseHandler->endAutoScroll();
    _currentMouseHandler->mouseReleaseEvent();
}

void DateBar::DateBar::mouseMoveEvent( QMouseEvent* event )
{
    showStatusBarTip( event->pos() );

    if ( (event->state() & LeftButton) == 0 )
        return;

    _currentMouseHandler->endAutoScroll();
    _currentMouseHandler->mouseMoveEvent( event->pos().x() );
}

QRect DateBar::DateBar::barAreaGeometry() const
{
    QRect barArea;
    barArea.setTopLeft( QPoint( borderArroundWidget, borderAboveHistogram ) );
    barArea.setRight( width() - borderArroundWidget - 2 * buttonWidth - 2*3 ); // 2 pixels between button and bar + 1 pixel as the pen is one pixel
    barArea.setHeight( _barHeight );
    return barArea;
}

int DateBar::DateBar::numberOfUnits() const
{
    return barAreaGeometry().width() / _barWidth -1 ;
}

void DateBar::DateBar::setHistogramBarSize( const QSize& size )
{
    _barWidth = size.width();
    _barHeight = size.height();
    _currentUnit = numberOfUnits()/2;
    Q_ASSERT( parentWidget() );
    updateGeometry();
    Q_ASSERT( parentWidget() );
    placeAndSizeButtons();
    redraw();
}

void DateBar::DateBar::setIncludeFuzzyCounts( bool b )
{
    _includeFuzzyCounts = b;
    redraw();
    if ( hasSelection() )
        emitRangeSelection( _selectionHandler->dateRange() );

    emit dateSelected( currentDateRange(), includeFuzzyCounts() );
}

ImageDate DateBar::DateBar::rangeAt( const QPoint& p )
{
    int unit = (p.x() - barAreaGeometry().x())/ _barWidth;
    return ImageDate( dateForUnit( unit ), dateForUnit(unit+1) );
}

bool DateBar::DateBar::includeFuzzyCounts() const
{
    return _includeFuzzyCounts;
}

void DateBar::DateBar::contextMenuEvent( QContextMenuEvent* event )
{
    if ( !_contextMenu ) {
        _contextMenu = new QPopupMenu( this );
        QAction* action = new QAction( i18n("Show Image Ranges"), 0, this );
        action->setToggleAction( true );
        action->addTo( _contextMenu );
        action->setOn( _includeFuzzyCounts );
        connect( action, SIGNAL( toggled( bool ) ), this, SLOT( setIncludeFuzzyCounts( bool ) ) );

        action = new QAction( i18n("Show Resolution Indicator"), 0, this );
        action->setToggleAction( true );
        action->addTo( _contextMenu );
        action->setOn( _showResolutionIndicator );
        connect( action, SIGNAL( toggled( bool ) ), this, SLOT( setShowResolutionIndicator( bool ) ) );
    }

    _contextMenu->exec( event->globalPos());
    event->accept();
}

QRect DateBar::DateBar::tickMarkGeometry() const
{
    QRect rect;
    rect.setTopLeft( barAreaGeometry().bottomLeft() );
    rect.setWidth( barAreaGeometry().width() );
    rect.setHeight( 12 );
    return rect;
}

void DateBar::DateBar::drawResolutionIndicator( QPainter& p, int* leftEdge )
{
    QRect rect = dateAreaGeometry();

    // For real small bars, we do not want to show the resolution.
    if ( rect.width() < 400 || !_showResolutionIndicator ) {
        *leftEdge = rect.right();
        return;
    }

    QString text = _currentHandler->unitText();
    int textWidth = QFontMetrics( font() ).width( text );
    int height = QFontMetrics( font() ).height();

    int endUnitPos = rect.right() - textWidth - arrowLength - 3;
    // Round to nearest unit mark
    endUnitPos = ( (endUnitPos-rect.left()) / _barWidth) * _barWidth + rect.left();
    int startUnitPos = endUnitPos - _barWidth;
    int midLine = rect.top() + height / 2;

    p.save();
    p.setPen( red );

    // draw arrows
    drawArrow( p, QPoint( startUnitPos - arrowLength, midLine ), QPoint( startUnitPos, midLine ) );
    drawArrow( p, QPoint( endUnitPos + arrowLength, midLine ), QPoint( endUnitPos, midLine ) );
    p.drawLine( startUnitPos, rect.top(), startUnitPos, rect.top()+height );
    p.drawLine( endUnitPos, rect.top(), endUnitPos, rect.top()+height );

    // draw text
    QFontMetrics fm( font() );
    p.drawText( endUnitPos + arrowLength + 3, rect.top(), fm.width(text), fm.height(), Qt::SingleLine, text );
    p.restore();

    *leftEdge = startUnitPos - arrowLength - 3;
}

QRect DateBar::DateBar::dateAreaGeometry() const
{
    QRect rect = tickMarkGeometry();
    rect.setTop( rect.bottom() + 2 );
    rect.setHeight( QFontMetrics( font() ).height() );
    return rect;
}

void DateBar::DateBar::drawArrow( QPainter& p, const QPoint& start, const QPoint& end )
{
    // PENDING(blackie) refactor with LineDraw::draw
    p.save();
    p.drawLine( start, end );

    QPoint diff = QPoint( end.x() - start.x(), end.y() - start.y() );
    double dx = diff.x();
    double dy = diff.y();

    if ( dx != 0 || dy != 0 ) {
        if( dy < 0 ) dx = -dx;
        double angle = acos(dx/sqrt( dx*dx+dy*dy ))*180./M_PI;
        if( dy < 0 ) angle += 180.;

        // angle is now the angle of the line.

        angle = angle + 180 - 15;
        p.translate( end.x(), end.y() );
        p.rotate( angle );
        p.drawLine( QPoint(0,0), QPoint( 10,0 ) );

        p.rotate( 30 );
        p.drawLine( QPoint(0,0), QPoint( 10,0 ) );
    }

    p.restore();

}

void DateBar::DateBar::setShowResolutionIndicator( bool b )
{
    _showResolutionIndicator = b;
    redraw();
}

void DateBar::DateBar::updateArrowState()
{
    _leftArrow->setEnabled( _dates->lowerLimit() <= dateForUnit( 0 ) );
    _rightArrow->setEnabled( _dates->upperLimit() > dateForUnit( numberOfUnits() ) );
}

ImageDate DateBar::DateBar::currentDateRange() const
{
    return ImageDate( dateForUnit( _currentUnit ), dateForUnit( _currentUnit+1 ) );
}

void DateBar::DateBar::showStatusBarTip( const QPoint& pos )
{
    ImageDate range = rangeAt( pos );
    ImageCount count = _dates->count( range );

    QString cnt;
    if ( count._rangeMatch != 0 && includeFuzzyCounts())
        cnt = i18n("%1 exact + %2 ranges = %3 images").arg( count._exact ).arg( count._rangeMatch ).arg( count._exact + count._rangeMatch );
    else
        cnt = i18n("%1 images").arg( count._exact );

    QString res = i18n("%1 to %2  %3").arg(range.start().toString()).arg(range.end().toString())
                  .arg(cnt);

    static QString lastTip = QString::null;
    if ( lastTip != res )
        emit toolTipInfo( res );
    lastTip = res;
}

void DateBar::DateBar::placeAndSizeButtons()
{
    _zoomIn->setFixedSize( buttonWidth, buttonWidth );
    _zoomOut->setFixedSize( buttonWidth, buttonWidth );
    _rightArrow->setFixedSize( QSize( buttonWidth, _barHeight ) );
    _leftArrow->setFixedSize( QSize( buttonWidth, _barHeight ) );

    _rightArrow->move( size().width() - _rightArrow->width() - borderArroundWidget, borderAboveHistogram );
    _leftArrow->move( _rightArrow->pos().x() - _leftArrow->width() -2 , borderAboveHistogram );

    int x = _leftArrow->pos().x();
    int y = height() - buttonWidth;
    _zoomOut->move( x, y );

    x = _rightArrow->pos().x();
    _zoomIn->move(x, y );


    _cancelSelection->setFixedSize( buttonWidth, buttonWidth );
    _cancelSelection->move( 0, y );
}

void DateBar::DateBar::keyPressEvent( QKeyEvent* event )
{
    int offset = 0;
    if ( event->key() == Key_Plus ) {
        if ( _tp != HourView )
            zoom(1);
        return;
    }
    if ( event->key() == Key_Minus ) {
        if ( _tp != DecadeView )
            zoom( -1 );
        return;
    }

    if ( event->key() == Key_Left )
        offset = -1;
    else if ( event->key() == Key_Right )
        offset = 1;
    else if ( event->key() == Key_PageDown )
        offset = -10;
    else if ( event->key() == Key_PageUp )
        offset = 10;
    else
        return;

    QDateTime newDate =dateForUnit( offset, _currentDate );
    if ( (offset < 0 && newDate >= _dates->lowerLimit()) ||
         ( offset > 0 && newDate <= _dates->upperLimit() ) ) {
        _currentDate = newDate;
        _currentUnit += offset;
        if ( _currentUnit < 0 )
            _currentUnit = 0;
        if ( _currentUnit > numberOfUnits() )
            _currentUnit = numberOfUnits();

        if ( ! currentSelection().includes( _currentDate ) )
            clearSelection();
    }
    redraw();
    emit dateSelected( currentDateRange(), includeFuzzyCounts() );
}

void DateBar::DateBar::focusInEvent( QFocusEvent* )
{
    redraw();
}

void DateBar::DateBar::focusOutEvent( QFocusEvent* )
{
    redraw();
}


int DateBar::DateBar::unitAtPos( int x ) const
{
    return ( x  - barAreaGeometry().left() )/_barWidth;
}

QDateTime DateBar::DateBar::dateForUnit( int unit, const QDateTime& offset ) const
{
    return _currentHandler->date( unit, offset );
}

bool DateBar::DateBar::isUnitSelected( int unit ) const
{
    QDateTime minDate = _selectionHandler->min();
    QDateTime maxDate = _selectionHandler->max();
    QDateTime date = dateForUnit( unit );
    return ( minDate <= date && date < maxDate && !minDate.isNull() );
}

bool DateBar::DateBar::hasSelection() const
{
    return !_selectionHandler->min().isNull();
}

ImageDate DateBar::DateBar::currentSelection() const
{
    return ImageDate(_selectionHandler->min(), _selectionHandler->max() );
}

void DateBar::DateBar::clearSelection()
{
    if ( _selectionHandler->hasSelection() ) {
        _selectionHandler->clearSelection();
        emit dateRangeCleared();
        redraw();
    }
}

void DateBar::DateBar::emitRangeSelection( const ImageDate&  range )
{
    emit dateRangeChange( range );
}

int DateBar::DateBar::unitForDate( const QDateTime& date ) const
{
    for ( int unit = 0; unit < numberOfUnits(); ++unit ) {
        if ( _currentHandler->date( unit ) <= date && date < _currentHandler->date( unit +1 ) )
            return unit;
    }
    return -1;
}

void DateBar::DateBar::emitDateSelected()
{
    emit dateSelected( currentDateRange(), includeFuzzyCounts() );
}

void DateBar::DateBar::wheelEvent( QWheelEvent * e )
{
    if ( e->delta() > 0 )
        scroll(1);
    else
        scroll(-1);
}

#include "DateBar.moc"
