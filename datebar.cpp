/* Copyright (C) 2003-2004 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "datebar.h"
#include <qdatetime.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qwmatrix.h>
#include <qfontmetrics.h>
#include "dateviewhandler.h"
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

DateBar::DateBar( QWidget* parent, const char* name )
    :QWidget( parent, name ), _currentHandler( &_yearViewHandler ), _tp(YearView),
     _includeFuzzyCounts( true ), _contextMenu(0), _showResolutionIndicator( true )
{
    setBackgroundMode( NoBackground );
    setMouseTracking( true );

    _barWidth = Options::instance()->histogramSize().width();
    _barHeight = Options::instance()->histogramSize().height();
    _rightArrow = new QToolButton( RightArrow, this );
    connect( _rightArrow, SIGNAL( clicked() ), this, SLOT( scrollRight() ) );

    _leftArrow = new QToolButton( LeftArrow, this );
    connect( _leftArrow, SIGNAL( clicked() ), this, SLOT( scrollLeft() ) );

    _zoomIn = new QToolButton( this );
    _zoomIn->setIconSet( KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "viewmag+" ), KIcon::Toolbar, 16 ) );
    connect( _zoomIn, SIGNAL( clicked() ), this, SLOT( zoomIn() ) );
    connect( this, SIGNAL(canZoomIn(bool)), _zoomIn, SLOT( setEnabled( bool ) ) );

    _zoomOut = new QToolButton( this );
    _zoomOut->setIconSet(  KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "viewmag-" ), KIcon::Toolbar, 16 ) );
    connect( _zoomOut, SIGNAL( clicked() ), this, SLOT( zoomOut() ) );
    connect( this, SIGNAL(canZoomOut(bool)), _zoomOut, SLOT( setEnabled( bool ) ) );

    placeAndSizeButtons();
    _autoScrollTimer = new QTimer( this );
    connect( _autoScrollTimer, SIGNAL( timeout() ), this, SLOT( autoScroll() ) );
}

QSize DateBar::sizeHint() const
{
    int height = QMAX( dateAreaGeometry().bottom() + borderArroundWidget,
                       _barHeight+ buttonWidth + 2* borderArroundWidget + 7 );
    return QSize( 800, height );
}

QSize DateBar::minimumSizeHint() const
{
     int height = QMAX( dateAreaGeometry().bottom() + borderArroundWidget,
                        _barHeight + buttonWidth + 2* borderArroundWidget + 7 );
     return QSize( 200, height );
}

void DateBar::paintEvent( QPaintEvent* /*event*/ )
{
    bitBlt( this, 0,0, &_buffer );
}

void DateBar::redraw()
{
    if ( _buffer.isNull() )
        return;

    QPainter p( &_buffer );
    p.setFont( font() );
    QColorGroup grp = palette().active();

    // Fill with background pixels
    p.save();
    // p.setPen( grp.dark() );
    p.setPen( NoPen );
    p.setBrush( grp.background() );
    p.drawRect( rect() );

    // Draw the area with histograms
    QRect barArea = barAreaGeometry();

    p.setPen( grp.dark() );
    p.setBrush( grp.base() );
    p.drawRect( barArea );
    p.restore();

    _currentHandler->init( _currentHandler->date( -_currentUnit, _currentDate ) );

    int right;
    drawResolutionIndicator( p, &right );
    QRect rect = dateAreaGeometry();
    rect.setRight( right );

    drawTickMarks( p, rect );
    drawHistograms( p );
    drawFocusRectagle( p );
    updateArrowState();
    repaint();
}

void DateBar::resizeEvent( QResizeEvent* event )
{
    placeAndSizeButtons();
    _buffer.resize( event->size() );
    _currentUnit = numberOfUnits()/2;
    redraw();
}

void DateBar::drawTickMarks( QPainter& p, const QRect& textRect )
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
        int h = rect.height();
        if ( _currentHandler->isMajorUnit( unit ) ) {
            QString text = _currentHandler->text( unit );
            int w = fm.width( text );
            p.setFont( f );
            if ( textRect.right() > x + w/2 )
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

void DateBar::setViewType( ViewType tp )
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

void DateBar::setDate( const QDateTime& date )
{
    _currentDate = date;
    redraw();
}

void DateBar::setImageRangeCollection( const ImageDateRangeCollection& dates )
{
    _dates = dates;
    redraw();
}

void DateBar::drawHistograms( QPainter& p)
{
    QRect rect = barAreaGeometry();
    p.save();
    p.setClipping( true );
    p.setClipRect( rect );
    p.setPen( NoPen );

    int unit = 0;
    int max = 0;
    for ( int x = rect.x(); x + _barWidth < rect.right(); x+=_barWidth, unit += 1 ) {
        ImageCount count = _dates.count( _currentHandler->date(unit), _currentHandler->date(unit+1).addSecs(-1) );
        int cnt = count._exact;
        if ( _includeFuzzyCounts )
            cnt += count._rangeMatch;
        max = QMAX( max, cnt  );
    }

    unit = 0;
    for ( int x = rect.x(); x  + _barWidth < rect.right(); x+=_barWidth, unit += 1 ) {
        ImageCount count = _dates.count( _currentHandler->date(unit), _currentHandler->date(unit+1).addSecs(-1) );
        int exact = 0;
        if ( max != 0 )
            exact = (int) ((double) (rect.height()-2) * count._exact / max );
        int range = 0;
        if ( _includeFuzzyCounts && max != 0 )
            range = (int) ((double) (rect.height()-2) * count._rangeMatch / max );

        p.setBrush( yellow );
        p.drawRect( x+1, rect.bottom()-range, _barWidth-2, range );
        p.setBrush( green );
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

void DateBar::scrollLeft()
{
    scroll( -1 );
}

void DateBar::scrollRight()
{
    scroll( 1 );
}

void DateBar::scroll( int units )
{
    _currentDate = _currentHandler->date( units, _currentDate );
    redraw();
}

void DateBar::drawFocusRectagle( QPainter& p)
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

    p.setBrush( red );
    p.setPen( red );
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

void DateBar::zoomIn()
{
    if ( _tp == HourView )
        return;
    zoom(+1);
}

void DateBar::zoomOut()
{
    if ( _tp == DecadeView )
        return;
    zoom(-1);
}

void DateBar::zoom( int factor )
{
    ViewType tp = (ViewType) (_tp+factor);
    setViewType( tp );
    emit canZoomIn( tp != HourView );
    emit canZoomOut( tp != DecadeView );
}

void DateBar::mousePressEvent( QMouseEvent* event )
{
    if ( (event->button() & LeftButton) == 0 ||  event->x() > barAreaGeometry().right() || event->x() < barAreaGeometry().left() )
        return;

    _movingBar = event->y() > barAreaGeometry().bottom();
    if ( _movingBar )
        _movementOffset = _currentUnit * _barWidth - ( event->pos().x() - barAreaGeometry().left() );
    else {
        _currentUnit = ( event->pos().x()  - barAreaGeometry().left() )/_barWidth;
        _currentDate = _currentHandler->date( _currentUnit );
        _movementOffset = 0;
    }
    emit dateSelected( currentDateRange(), includeFuzzyCounts() );
    showStatusBarTip( event->pos() );
    redraw();
}

void DateBar::mouseReleaseEvent( QMouseEvent* )
{
    endAutoScroll();
}

void DateBar::mouseMoveEvent( QMouseEvent* event )
{
    showStatusBarTip( event->pos() );

    if ( (event->state() & LeftButton) == 0 )
        return;

    endAutoScroll();
    doScroll( event->pos().x() );
    emit dateSelected( currentDateRange(), includeFuzzyCounts() );
}

QRect DateBar::barAreaGeometry() const
{
    QRect barArea;
    barArea.setTopLeft( QPoint( borderArroundWidget, borderAboveHistogram ) );
    barArea.setRight( width() - borderArroundWidget - 2 * buttonWidth - 2*3 ); // 2 pixels between button and bar + 1 pixel as the pen is one pixel
    barArea.setHeight( _barHeight );
    return barArea;
}

int DateBar::numberOfUnits() const
{
    return barAreaGeometry().width() / _barWidth -1 ;
}

void DateBar::setHistogramBarSize( const QSize& size )
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

void DateBar::setIncludeFuzzyCounts( bool b )
{
    _includeFuzzyCounts = b;
    redraw();
    emit dateSelected( currentDateRange(), includeFuzzyCounts() );
}

ImageDateRange DateBar::rangeAt( const QPoint& p )
{
    int unit = (p.x() - barAreaGeometry().x())/ _barWidth;
    return ImageDateRange( _currentHandler->date( unit ), _currentHandler->date(unit+1) );
}

bool DateBar::includeFuzzyCounts() const
{
    return _includeFuzzyCounts;
}

void DateBar::contextMenuEvent( QContextMenuEvent* event )
{
    if ( !_contextMenu ) {
        _contextMenu = new QPopupMenu( this );
        QAction* action = new QAction( i18n("Show Image Ranged"), 0, this );
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

QRect DateBar::tickMarkGeometry() const
{
    QRect rect;
    rect.setTopLeft( barAreaGeometry().bottomLeft() );
    rect.setWidth( barAreaGeometry().width() );
    rect.setHeight( 12 );
    return rect;
}

void DateBar::drawResolutionIndicator( QPainter& p, int* leftEdge )
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

QRect DateBar::dateAreaGeometry() const
{
    QRect rect = tickMarkGeometry();
    rect.setTop( rect.bottom() + 2 );
    rect.setHeight( QFontMetrics( font() ).height() );
    return rect;
}

void DateBar::drawArrow( QPainter& p, const QPoint& start, const QPoint& end )
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

void DateBar::setShowResolutionIndicator( bool b )
{
    _showResolutionIndicator = b;
    redraw();
}

void DateBar::startAutoScroll()
{
    _autoScrollTimer->start( 100 );
}

void DateBar::endAutoScroll()
{
    _autoScrollTimer->stop();
}

void DateBar::autoScroll()
{
    doScroll( mapFromGlobal( QCursor::pos() ).x() );
}

void DateBar::doScroll( int x )
{
    int oldUnit = _currentUnit;
    _currentUnit = ( x + _movementOffset - barAreaGeometry().left() )/_barWidth;

    // Don't scroll further down than the last image
    // We use oldUnit here, to ensure that we scroll all the way to the end
    // better scroll a bit over than not all the way.
    if ( !_movingBar &&
         ( (_currentUnit > oldUnit &&
            _currentHandler->date( oldUnit ) > _dates.upperLimit() ) ||
           ( _currentUnit < oldUnit &&
             _currentHandler->date( oldUnit ) < _dates.lowerLimit() ) ) ||
         _movingBar &&
         ( (_currentUnit > oldUnit &&
            _currentHandler->date( 0 ) < _dates.lowerLimit() ) ||
           ( _currentUnit < oldUnit &&
             _currentHandler->date( numberOfUnits() ) > _dates.upperLimit() ) )  ) {
        _currentUnit = oldUnit;
        return;
    }

    if ( _movingBar ) {
        if ( _currentUnit < 0 ) {
            _currentDate = _currentHandler->date( - _currentUnit );
            _currentUnit = 0;
            _movementOffset = barAreaGeometry().left() - x;
        }
        else if ( _currentUnit > numberOfUnits() ) {
            int diff = numberOfUnits() - _currentUnit;
            _currentDate = _currentHandler->date( numberOfUnits()+diff );
            _currentUnit = numberOfUnits();
            _movementOffset = (numberOfUnits()*_barWidth ) - x + _barWidth/ 2  ;
        }
    }
    else {
        static double rest = 0;
        if ( _currentUnit < 0 || _currentUnit > numberOfUnits() ) {
            // Slow down scrolling outside date bar.
            double newUnit = oldUnit + ( _currentUnit - oldUnit ) / 4.0 + rest;
            _currentUnit = (int) floor( newUnit );
            rest = newUnit - _currentUnit;
            startAutoScroll();
        }

        _currentDate = _currentHandler->date( _currentUnit );
        _currentUnit = QMAX( _currentUnit, 0 );
        _currentUnit = QMIN( _currentUnit, numberOfUnits() );
    }

    redraw();
}

void DateBar::updateArrowState()
{
    _leftArrow->setEnabled( _dates.lowerLimit() <= _currentHandler->date( 0 ) );
    _rightArrow->setEnabled( _dates.upperLimit() > _currentHandler->date( numberOfUnits() ) );
}

ImageDateRange DateBar::currentDateRange() const
{
    return ImageDateRange( ImageDate( _currentDate ), ImageDate( _currentHandler->date( _currentUnit+1 ) ) );
}

void DateBar::showStatusBarTip( const QPoint& pos )
{
    ImageDateRange range = rangeAt( pos );
    ImageCount count = _dates.count( range.start(), range.end().max().addSecs(-1) );

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

void DateBar::placeAndSizeButtons()
{
    _zoomIn->setFixedSize( buttonWidth, buttonWidth );
    _zoomOut->setFixedSize( buttonWidth, buttonWidth );
    _rightArrow->setFixedSize( QSize( buttonWidth, _barHeight ) );
    _leftArrow->setFixedSize( QSize( buttonWidth, _barHeight ) );

    _rightArrow->move( size().width() - _rightArrow->width() - borderArroundWidget, borderAboveHistogram );
    _leftArrow->move( _rightArrow->pos().x() - _leftArrow->width() -2 , borderAboveHistogram );

    int x = _leftArrow->pos().x();
    int y = _rightArrow->geometry().bottom() + 3;
    _zoomOut->move( x, y );

    x = _rightArrow->pos().x();
    _zoomIn->move(x, y );
}

void DateBar::showEvent( QShowEvent *)
{
    _currentDate = QDateTime::currentDateTime();
    _currentUnit = numberOfUnits();
    redraw();
}
