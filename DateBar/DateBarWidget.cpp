/* Copyright (C) 2003-2018 Jesper K. Pedersen <blackie@kde.org>

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

#include "DateBarWidget.h"

#include <math.h>

#include <QAction>
#include <QContextMenuEvent>
#include <QDateTime>
#include <QFontMetrics>
#include <QIcon>
#include <QLocale>
#include <QMenu>
#include <QPainter>
#include <QToolButton>
#include <QGuiApplication>

#include <KLocalizedString>

#include <DB/ImageDateCollection.h>
#include "MouseHandler.h"
#include "Settings/SettingsData.h"

namespace {
constexpr int BORDER_ABOVE_HISTOGRAM = 4;
constexpr int BORDER_AROUND_WIDGET = 0;
constexpr int BUTTON_WIDTH = 22;
constexpr int ARROW_LENGTH = 20;

constexpr int SCROLL_AMOUNT = 1;
constexpr int SCROLL_ACCELERATION = 10;
}

/**
 * \class DateBar::DateBarWidget
 * \brief This class represents the date bar at the bottom of the main window.
 *
 * The mouse interaction is handled by the classes which inherits \ref DateBar::MouseHandler, while the logic for
 * deciding the length (in minutes, hours, days, etc) are handled by subclasses of \ref DateBar::ViewHandler.
 */

DateBar::DateBarWidget::DateBarWidget( QWidget* parent )
    :QWidget( parent ), m_currentHandler( &m_yearViewHandler ),m_tp(YearView), m_currentMouseHandler(nullptr),
     m_currentUnit(0), m_currentDate( QDateTime::currentDateTime() ),m_includeFuzzyCounts( true ), m_contextMenu(nullptr),
     m_showResolutionIndicator( true ), m_doAutomaticRangeAdjustment( true )
{
    setMouseTracking( true );
    setFocusPolicy( Qt::StrongFocus );

    m_barWidth = Settings::SettingsData::instance()->histogramSize().width();
    m_barHeight = Settings::SettingsData::instance()->histogramSize().height();
    m_rightArrow = new QToolButton( this );
    m_rightArrow->setArrowType( Qt::RightArrow );
    m_rightArrow->setAutoRepeat( true );
    connect( m_rightArrow, SIGNAL(clicked()), this, SLOT(scrollRight()) );

    m_leftArrow = new QToolButton( this );
    m_leftArrow->setArrowType( Qt::LeftArrow );
    m_leftArrow->setAutoRepeat( true );
    connect( m_leftArrow, SIGNAL(clicked()), this, SLOT(scrollLeft()) );

    m_zoomIn = new QToolButton( this );
    m_zoomIn->setIcon( QIcon::fromTheme( QStringLiteral( "zoom-in" ) ) );
    connect( m_zoomIn, SIGNAL(clicked()), this, SLOT(zoomIn()) );
    connect( this, SIGNAL(canZoomIn(bool)), m_zoomIn, SLOT(setEnabled(bool)) );

    m_zoomOut = new QToolButton( this );
    m_zoomOut->setIcon(  QIcon::fromTheme( QStringLiteral( "zoom-out" ) ) );
    connect( m_zoomOut, SIGNAL(clicked()), this, SLOT(zoomOut()) );
    connect( this, SIGNAL(canZoomOut(bool)), m_zoomOut, SLOT(setEnabled(bool)) );

    m_cancelSelection = new QToolButton( this );
    m_cancelSelection->setIcon( QIcon( QStringLiteral( "dialog-close" ) ) );
    connect( m_cancelSelection, SIGNAL(clicked()), this, SLOT(clearSelection()) );
    m_cancelSelection->setEnabled( false );
    m_cancelSelection->setToolTip( i18n("Widen selection to include all images and videos again") );

    placeAndSizeButtons();

    m_focusItemDragHandler = new FocusItemDragHandler( this );
    m_barDragHandler = new BarDragHandler( this );
    m_selectionHandler = new SelectionHandler( this );

    setWhatsThis( i18nc( "@info", "<title>The date bar</title>"
            "<para>The date bar gives you an overview of the approximate number of images taken in a given time frame."
            "Time units are shown on the <emphasis>timeline</emphasis>. Above it, a histogram indicates the number of images for that time range.</para>"
            "<para>You can interact with the date bar in several ways:<list>"
            "<item>Zoom in or out by using the +/- buttons or Ctrl + scrollwheel.</item>"
            "<item>Scroll the timeline either by using the arrow buttons or the scroll wheel, or by dragging it using the middle mouse button.</item>"
            "<item>Restrict the current view to a given time frame by clicking below the timeline and marking the time frame.</item>"
            "<item>Clicking on the timeline sets the <emphasis>focus</emphasis> for the thumbnail view, i.e. jumps to the first thumbnail of the time unit in focus.</item>"
            "</list></para>") );
    setToolTip( whatsThis() );
}

QSize DateBar::DateBarWidget::sizeHint() const
{
    int height = qMax( dateAreaGeometry().bottom() + BORDER_AROUND_WIDGET,
                       m_barHeight+ BUTTON_WIDTH + 2* BORDER_AROUND_WIDGET + 7 );
    return QSize( 800, height );
}

QSize DateBar::DateBarWidget::minimumSizeHint() const
{
     int height = qMax( dateAreaGeometry().bottom() + BORDER_AROUND_WIDGET,
                        m_barHeight + BUTTON_WIDTH + 2* BORDER_AROUND_WIDGET + 7 );
     return QSize( 200, height );
}

void DateBar::DateBarWidget::paintEvent( QPaintEvent* /*event*/ )
{
    QPainter painter( this );
    painter.drawPixmap( 0,0, m_buffer );
}

void DateBar::DateBarWidget::redraw()
{
    if ( m_buffer.isNull() )
        return;

    QPainter p( &m_buffer );
    p.setRenderHint( QPainter::Antialiasing );
    p.setFont( font() );

    // Fill with background pixels
    p.save();
    p.setPen( Qt::NoPen );
    p.setBrush( palette().brush( QPalette::Background ) );
    p.drawRect( rect() );

    if (!m_dates )
        return;

    // Draw the area with histograms
    QRect barArea = barAreaGeometry();

    p.setPen( palette().color( QPalette::Dark ) );
    p.setBrush( palette().brush( QPalette::Base ) );
    p.drawRect( barArea );
    p.restore();

    m_currentHandler->init( dateForUnit( -m_currentUnit, m_currentDate ) );

    int right;
    drawResolutionIndicator( p, &right );
    QRect rect = dateAreaGeometry();
    rect.setRight( right );
    rect.setLeft( rect.left() + BUTTON_WIDTH + 2 );

    drawTickMarks( p, rect );
    drawHistograms( p );
    drawFocusRectagle( p );
    updateArrowState();
    repaint();
}

void DateBar::DateBarWidget::resizeEvent( QResizeEvent* event )
{
    placeAndSizeButtons();
    m_buffer = QPixmap( event->size() );
    m_currentUnit = numberOfUnits()/2;
    redraw();
}

void DateBar::DateBarWidget::drawTickMarks( QPainter& p, const QRect& textRect )
{
    QRect rect = tickMarkGeometry();
    p.save();
    p.setPen( QPen( palette().color( QPalette::Text) , 1 ) );

    QFont f( font() );
    QFontMetrics fm(f);
    int fontHeight = fm.height();
    int unit = 0;
    QRect clip = rect;
    clip.setHeight( rect.height() + 2 + fontHeight );
    clip.setLeft( clip.left() + 2 );
    clip.setRight( clip.right() -2 );
    p.setClipRect( clip );

    for ( int x = rect.x(); x < rect.right(); x+=m_barWidth, unit += 1 ) {
        // draw selection indication
        p.save();
        p.setPen( Qt::NoPen );
        p.setBrush( palette().brush( QPalette::Highlight ) );
        QDateTime date = dateForUnit( unit );
        if ( isUnitSelected( unit ) )
            p.drawRect( QRect( x, rect.top(), m_barWidth, rect.height() ) );
        p.restore();

        // draw tickmarks
        int h = rect.height();
        if ( m_currentHandler->isMajorUnit( unit ) ) {
            QString text = m_currentHandler->text( unit );
            int w = fm.width( text );
            p.setFont( f );
            if ( textRect.right() >  x + w/2 && textRect.left() < x - w/2)
                p.drawText( x - w/2, textRect.top(), w, fontHeight, Qt::TextSingleLine, text );
        }
        else if ( m_currentHandler->isMidUnit( unit ) )
            h = (int) ( 2.0/3*rect.height());
        else
            h = (int) ( 1.0/3*rect.height());

        p.drawLine( x, rect.top(), x, rect.top() + h );
    }

    p.restore();
}

void DateBar::DateBarWidget::setViewType( ViewType tp, bool redrawNow )
{
    setViewHandlerForType(tp);
    if ( redrawNow )
        redraw();
    m_tp = tp;
}

void DateBar::DateBarWidget::setViewHandlerForType( ViewType tp )
{
    switch ( tp ) {
    case DecadeView: m_currentHandler = &m_decadeViewHandler; break;
    case YearView: m_currentHandler = &m_yearViewHandler; break;
    case MonthView: m_currentHandler = &m_monthViewHandler; break;
    case WeekView: m_currentHandler = &m_weekViewHandler; break;
    case DayView: m_currentHandler = &m_dayViewHandler; break;
    case HourView: m_currentHandler = &m_hourViewHandler; break;
    case MinuteView: m_currentHandler = &m_minuteViewHandler; break;
    }
}

void DateBar::DateBarWidget::setDate( const QDateTime& date )
{
    m_currentDate = date;
    if ( hasSelection() ) {
        if ( currentSelection().start() > m_currentDate )
            m_currentDate = currentSelection().start();
        if ( currentSelection().end() < m_currentDate )
            m_currentDate = currentSelection().end();
    }

    if ( unitForDate( m_currentDate ) != -1 )
        m_currentUnit = unitForDate( m_currentDate );

    redraw();
}

void DateBar::DateBarWidget::setImageDateCollection( const QExplicitlySharedDataPointer<DB::ImageDateCollection>& dates )
{
    m_dates = dates;
    if ( m_doAutomaticRangeAdjustment && m_dates && ! m_dates->lowerLimit().isNull())
    {
        QDateTime start = m_dates->lowerLimit();
        QDateTime end = m_dates->upperLimit();
        if ( end.isNull() )
            end = QDateTime::currentDateTime();

        m_currentDate =  start;
        m_currentUnit = 0;
        // select suitable timeframe:
        setViewType( MinuteView, false );
        m_currentHandler->init(start);
        while ( m_tp != DecadeView && end > dateForUnit( numberOfUnits() ) )
        {
            m_tp = (ViewType) (m_tp-1);
            setViewHandlerForType( m_tp );
            m_currentHandler->init(start);
        }
        // center range in datebar:
        int units = unitForDate( end );
        if ( units != -1 )
        {
            m_currentUnit = (numberOfUnits() - units )/2;
        }
    }
    redraw();
}


void DateBar::DateBarWidget::drawHistograms( QPainter& p)
{
    QRect rect = barAreaGeometry();
    p.save();
    p.setClipping( true );
    p.setClipRect( rect );
    p.setPen( Qt::NoPen );

    int unit = 0;
    int max = 0;
    for ( int x = rect.x(); x + m_barWidth < rect.right(); x+=m_barWidth, unit += 1 ) {
        DB::ImageCount count = m_dates->count( rangeForUnit(unit) );
        int cnt = count.mp_exact;
        if ( m_includeFuzzyCounts )
            cnt += count.mp_rangeMatch;
        max = qMax( max, cnt  );
    }

    // Calculate the font size for the largest number.
    QFont f = font();
    bool fontFound = false;
    for ( int i = f.pointSize(); i >= 6; i-=2 ) {
        f.setPointSize( i );
        int w = QFontMetrics(f).width( QString::number( max ) );
        if ( w < rect.height() - 6 ) {
            p.setFont(f);
            fontFound = true;
            break;
        }
    }

    unit = 0;
    for ( int x = rect.x(); x  + m_barWidth < rect.right(); x+=m_barWidth, unit += 1 ) {
        DB::ImageCount count = m_dates->count( rangeForUnit(unit) );
        int exact = 0;
        if ( max != 0 )
            exact = (int) ((double) (rect.height()-2) * count.mp_exact / max );
        int range = 0;
        if ( m_includeFuzzyCounts && max != 0 )
            range = (int) ((double) (rect.height()-2) * count.mp_rangeMatch / max );

        Qt::BrushStyle style = Qt::SolidPattern;
        if ( !isUnitSelected( unit ) && hasSelection() )
            style= Qt::Dense5Pattern;

        p.setBrush( QBrush( Qt::yellow, style ) );
        p.drawRect( x+1, rect.bottom()-range, m_barWidth-2, range );
        p.setBrush( QBrush( Qt::green, style ) );
        p.drawRect( x+1, rect.bottom()-range-exact, m_barWidth-2, exact );

        // Draw the numbers, if they fit.
        if (fontFound) {
            int tot = count.mp_exact;
            if ( m_includeFuzzyCounts )
                tot += count.mp_rangeMatch;
            p.save();
            p.translate( x+m_barWidth-3, rect.bottom()-2 );
            p.rotate( -90 );
            int w = QFontMetrics(f).width( QString::number( tot ) );
            if ( w < exact+range-2 ) {
                p.setPen( Qt::black );
                p.drawText( 0,0, QString::number( tot ) );
            }
            p.restore();
        }
    }

    p.restore();
}

void DateBar::DateBarWidget::scrollLeft()
{
    int scrollAmount = -SCROLL_AMOUNT;
    if ( QGuiApplication::keyboardModifiers().testFlag( Qt::ShiftModifier ) )
        scrollAmount *= SCROLL_ACCELERATION;
    scroll( scrollAmount );
}

void DateBar::DateBarWidget::scrollRight()
{
    int scrollAmount = SCROLL_AMOUNT;
    if ( QGuiApplication::keyboardModifiers().testFlag( Qt::ShiftModifier ) )
        scrollAmount *= SCROLL_ACCELERATION;
    scroll( scrollAmount );
}

void DateBar::DateBarWidget::scroll( int units )
{
    m_currentDate = dateForUnit( units, m_currentDate );
    redraw();
    emit dateSelected( currentDateRange(), includeFuzzyCounts() );
}

void DateBar::DateBarWidget::drawFocusRectagle( QPainter& p)
{
    QRect rect = barAreaGeometry();
    p.save();
    int x = rect.left() + m_currentUnit*m_barWidth;
    QRect inner( QPoint(x-1, BORDER_ABOVE_HISTOGRAM),
                 QPoint( x + m_barWidth, BORDER_ABOVE_HISTOGRAM + m_barHeight - 1 ) );

    p.setPen( QPen( palette().color( QPalette::Dark ), 1 ) );

    // Inner rect
    p.drawRect( inner );
    QRect outer = inner;
    outer.adjust( -2, -2, 2, 2 );

    // Outer rect
    QRegion region = outer;
    region -= inner;
    p.setClipping( true );
    p.setClipRegion( region );

    QColor col = Qt::gray;
    if ( !hasFocus() )
        col = Qt::white;

    p.setBrush( col );
    p.setPen( col );
    p.drawRect( outer );

    // Shadow below
    QRect shadow = outer;
    shadow.adjust( -1,-1, 1, 1 );
    region = shadow;
    region -= outer;
    p.setPen( palette().color( QPalette::Shadow ) );
    p.setClipRegion( region );
    p.drawRect( shadow );

    // Light above
    QRect hide = shadow;
    hide.translate( 1, 1 );
    region = shadow;
    region -= hide;
    p.setPen( palette().color( QPalette::Light ) );
    p.setClipRegion( region );
    p.drawRect( shadow );

    p.restore();
}

void DateBar::DateBarWidget::zoomIn()
{
    if ( m_tp == MinuteView )
        return;
    zoom(+1);
}

void DateBar::DateBarWidget::zoomOut()
{
    if ( m_tp == DecadeView )
        return;
    zoom(-1);
}

void DateBar::DateBarWidget::zoom( int factor )
{
    ViewType tp = (ViewType) (m_tp+factor);
    setViewType( tp );
    emit canZoomIn( tp != MinuteView );
    emit canZoomOut( tp != DecadeView );
}

void DateBar::DateBarWidget::mousePressEvent( QMouseEvent* event )
{
    if ( (event->button() & ( Qt::MidButton | Qt::LeftButton)) == 0 ||  event->x() > barAreaGeometry().right() || event->x() < barAreaGeometry().left() )
        return;

    if ( (event->button() & Qt::MidButton)
            || event->modifiers() & Qt::ControlModifier ) {
        m_currentMouseHandler = m_barDragHandler;
    }
    else {
        bool onBar = event->y() > barAreaGeometry().bottom();
        if ( onBar )
            m_currentMouseHandler = m_selectionHandler;
        else {
            m_currentMouseHandler= m_focusItemDragHandler;
        }
    }
    m_currentMouseHandler->mousePressEvent( event->x() );
    m_cancelSelection->setEnabled( hasSelection() );
    emit dateSelected( currentDateRange(), includeFuzzyCounts() );
    showStatusBarTip( event->pos() );
    redraw();
}

void DateBar::DateBarWidget::mouseReleaseEvent( QMouseEvent* )
{
    if ( m_currentMouseHandler == nullptr )
        return;

    m_currentMouseHandler->endAutoScroll();
    m_currentMouseHandler->mouseReleaseEvent();
    m_currentMouseHandler = nullptr;
}

void DateBar::DateBarWidget::mouseMoveEvent( QMouseEvent* event )
{
    if ( m_currentMouseHandler == nullptr)
        return;

    showStatusBarTip( event->pos() );

    if ( (event->buttons() & ( Qt::MidButton | Qt::LeftButton)) == 0 )
        return;

    m_currentMouseHandler->endAutoScroll();
    m_currentMouseHandler->mouseMoveEvent( event->pos().x() );
}

QRect DateBar::DateBarWidget::barAreaGeometry() const
{
    QRect barArea;
    barArea.setTopLeft( QPoint( BORDER_AROUND_WIDGET, BORDER_ABOVE_HISTOGRAM ) );
    barArea.setRight( width() - BORDER_AROUND_WIDGET - 2 * BUTTON_WIDTH - 2*3 ); // 2 pixels between button and bar + 1 pixel as the pen is one pixel
    barArea.setHeight( m_barHeight );
    return barArea;
}

int DateBar::DateBarWidget::numberOfUnits() const
{
    return barAreaGeometry().width() / m_barWidth -1 ;
}

void DateBar::DateBarWidget::setHistogramBarSize( const QSize& size )
{
    m_barWidth = size.width();
    m_barHeight = size.height();
    m_currentUnit = numberOfUnits()/2;
    Q_ASSERT( parentWidget() );
    updateGeometry();
    Q_ASSERT( parentWidget() );
    placeAndSizeButtons();
    redraw();
}

void DateBar::DateBarWidget::setIncludeFuzzyCounts( bool b )
{
    m_includeFuzzyCounts = b;
    redraw();
    if ( hasSelection() )
        emitRangeSelection( m_selectionHandler->dateRange() );

    emit dateSelected( currentDateRange(), includeFuzzyCounts() );
}

DB::ImageDate DateBar::DateBarWidget::rangeAt( const QPoint& p )
{
    int unit = (p.x() - barAreaGeometry().x())/ m_barWidth;
    return rangeForUnit( unit );
}


DB::ImageDate DateBar::DateBarWidget::rangeForUnit( int unit )
{
    // Note on the use of setTimeSpec.
    // It came to my attention that addSec would create a QDateTime with internal type LocalStandard, while all the others would have type LocalUnknown,
    // this resulted in that QDateTime::operator<() would call getUTC(), which took 90% of the time for populating the datebar.
    QDateTime toUnit = dateForUnit(unit+1).addSecs(-1);
    toUnit.setTimeSpec( Qt::LocalTime);
    return DB::ImageDate( dateForUnit(unit), toUnit );
}

bool DateBar::DateBarWidget::includeFuzzyCounts() const
{
    return m_includeFuzzyCounts;
}

void DateBar::DateBarWidget::contextMenuEvent( QContextMenuEvent* event )
{
    if ( !m_contextMenu ) {
        m_contextMenu = new QMenu( this );
        QAction* action = new QAction( i18n("Show Ranges"), this );
        action->setCheckable( true );
        m_contextMenu->addAction(action);
        action->setChecked( m_includeFuzzyCounts );
        connect( action, SIGNAL(toggled(bool)), this, SLOT(setIncludeFuzzyCounts(bool)) );

        action = new QAction( i18n("Show Resolution Indicator"), this );
        action->setCheckable( true );
        m_contextMenu->addAction(action);
        action->setChecked( m_showResolutionIndicator );
        connect( action, SIGNAL(toggled(bool)), this, SLOT(setShowResolutionIndicator(bool)) );
    }

    m_contextMenu->exec( event->globalPos());
    event->setAccepted(true);
}

QRect DateBar::DateBarWidget::tickMarkGeometry() const
{
    QRect rect;
    rect.setTopLeft( barAreaGeometry().bottomLeft() );
    rect.setWidth( barAreaGeometry().width() );
    rect.setHeight( 12 );
    return rect;
}

void DateBar::DateBarWidget::drawResolutionIndicator( QPainter& p, int* leftEdge )
{
    QRect rect = dateAreaGeometry();

    // For real small bars, we do not want to show the resolution.
    if ( rect.width() < 400 || !m_showResolutionIndicator ) {
        *leftEdge = rect.right();
        return;
    }

    QString text = m_currentHandler->unitText();
    int textWidth = QFontMetrics( font() ).width( text );
    int height = QFontMetrics( font() ).height();

    int endUnitPos = rect.right() - textWidth - ARROW_LENGTH - 3;
    // Round to nearest unit mark
    endUnitPos = ( (endUnitPos-rect.left()) / m_barWidth) * m_barWidth + rect.left();
    int startUnitPos = endUnitPos - m_barWidth;
    int midLine = rect.top() + height / 2;

    p.save();
    p.setPen( Qt::red );

    // draw arrows
    drawArrow( p, QPoint( startUnitPos - ARROW_LENGTH, midLine ), QPoint( startUnitPos, midLine ) );
    drawArrow( p, QPoint( endUnitPos + ARROW_LENGTH, midLine ), QPoint( endUnitPos, midLine ) );
    p.drawLine( startUnitPos, rect.top(), startUnitPos, rect.top()+height );
    p.drawLine( endUnitPos, rect.top(), endUnitPos, rect.top()+height );

    // draw text
    QFontMetrics fm( font() );
    p.drawText( endUnitPos + ARROW_LENGTH + 3, rect.top(), fm.width(text), fm.height(), Qt::TextSingleLine, text );
    p.restore();

    *leftEdge = startUnitPos - ARROW_LENGTH - 3;
}

QRect DateBar::DateBarWidget::dateAreaGeometry() const
{
    QRect rect = tickMarkGeometry();
    rect.setTop( rect.bottom() + 2 );
    rect.setHeight( QFontMetrics( font() ).height() );
    return rect;
}

void DateBar::DateBarWidget::drawArrow( QPainter& p, const QPoint& start, const QPoint& end )
{
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

void DateBar::DateBarWidget::setShowResolutionIndicator( bool b )
{
    m_showResolutionIndicator = b;
    redraw();
}

void DateBar::DateBarWidget::setAutomaticRangeAdjustment( bool b )
{
    m_doAutomaticRangeAdjustment = b;
}

void DateBar::DateBarWidget::updateArrowState()
{
    m_leftArrow->setEnabled( m_dates->lowerLimit() <= dateForUnit( 0 ) );
    m_rightArrow->setEnabled( m_dates->upperLimit() > dateForUnit( numberOfUnits() ) );
}

DB::ImageDate DateBar::DateBarWidget::currentDateRange() const
{
    return DB::ImageDate( dateForUnit( m_currentUnit ), dateForUnit( m_currentUnit+1 ) );
}

void DateBar::DateBarWidget::showStatusBarTip( const QPoint& pos )
{
    DB::ImageDate range = rangeAt( pos );
    DB::ImageCount count = m_dates->count( range );

    QString cnt;
    if ( count.mp_rangeMatch != 0 && includeFuzzyCounts())
        cnt = i18ncp("@info:status images that fall in the given date range"
               ,"1 exact", "%1 exact", count.mp_exact)
                + i18ncp("@info:status additional images captured in a date range that overlaps with the given date range,"
                        ," + 1 range", " + %1 ranges", count.mp_rangeMatch)
                + i18ncp("@info:status total image count"," = 1 total", " = %1 total",  count.mp_exact + count.mp_rangeMatch );
    else
        cnt = i18ncp("@info:status image count","%1 image/video","%1 images/videos", count.mp_exact );

    QString res = i18nc("@info:status Time range vs. image count (e.g. 'Jun 2012 | 4 images/videos').","%1 | %2", range.toString(), cnt);

    static QString lastTip;
    if ( lastTip != res )
        emit toolTipInfo( res );
    lastTip = res;
}

void DateBar::DateBarWidget::placeAndSizeButtons()
{
    m_zoomIn->setFixedSize( BUTTON_WIDTH, BUTTON_WIDTH );
    m_zoomOut->setFixedSize( BUTTON_WIDTH, BUTTON_WIDTH );
    m_rightArrow->setFixedSize( QSize( BUTTON_WIDTH, m_barHeight ) );
    m_leftArrow->setFixedSize( QSize( BUTTON_WIDTH, m_barHeight ) );

    m_rightArrow->move( size().width() - m_rightArrow->width() - BORDER_AROUND_WIDGET, BORDER_ABOVE_HISTOGRAM );
    m_leftArrow->move( m_rightArrow->pos().x() - m_leftArrow->width() -2 , BORDER_ABOVE_HISTOGRAM );

    int x = m_leftArrow->pos().x();
    int y = height() - BUTTON_WIDTH;
    m_zoomOut->move( x, y );

    x = m_rightArrow->pos().x();
    m_zoomIn->move(x, y );


    m_cancelSelection->setFixedSize( BUTTON_WIDTH, BUTTON_WIDTH );
    m_cancelSelection->move( 0, y );
}

void DateBar::DateBarWidget::keyPressEvent( QKeyEvent* event )
{
    int offset = 0;
    if ( event->key() == Qt::Key_Plus ) {
        if ( m_tp != MinuteView )
            zoom(1);
        return;
    }
    if ( event->key() == Qt::Key_Minus ) {
        if ( m_tp != DecadeView )
            zoom( -1 );
        return;
    }

    if ( event->key() == Qt::Key_Left )
        offset = -1;
    else if ( event->key() == Qt::Key_Right )
        offset = 1;
    else if ( event->key() == Qt::Key_PageDown )
        offset = -10;
    else if ( event->key() == Qt::Key_PageUp )
        offset = 10;
    else
        return;

    QDateTime newDate =dateForUnit( offset, m_currentDate );
    if ( (offset < 0 && newDate >= m_dates->lowerLimit()) ||
         ( offset > 0 && newDate <= m_dates->upperLimit() ) ) {
        m_currentDate = newDate;
        m_currentUnit += offset;
        if ( m_currentUnit < 0 )
            m_currentUnit = 0;
        if ( m_currentUnit > numberOfUnits() )
            m_currentUnit = numberOfUnits();

        if ( ! currentSelection().includes( m_currentDate ) )
            clearSelection();
    }
    redraw();
    emit dateSelected( currentDateRange(), includeFuzzyCounts() );
}

void DateBar::DateBarWidget::focusInEvent( QFocusEvent* )
{
    redraw();
}

void DateBar::DateBarWidget::focusOutEvent( QFocusEvent* )
{
    redraw();
}


int DateBar::DateBarWidget::unitAtPos( int x ) const
{
    return ( x  - barAreaGeometry().left() )/m_barWidth;
}

QDateTime DateBar::DateBarWidget::dateForUnit( int unit, const QDateTime& offset ) const
{
    return m_currentHandler->date( unit, offset );
}

bool DateBar::DateBarWidget::isUnitSelected( int unit ) const
{
    QDateTime minDate = m_selectionHandler->min();
    QDateTime maxDate = m_selectionHandler->max();
    QDateTime date = dateForUnit( unit );
    return ( minDate <= date && date < maxDate && !minDate.isNull() );
}

bool DateBar::DateBarWidget::hasSelection() const
{
    return !m_selectionHandler->min().isNull();
}

DB::ImageDate DateBar::DateBarWidget::currentSelection() const
{
    return DB::ImageDate(m_selectionHandler->min(), m_selectionHandler->max() );
}

void DateBar::DateBarWidget::clearSelection()
{
    if ( m_selectionHandler->hasSelection() ) {
        m_selectionHandler->clearSelection();
        emit dateRangeCleared();
        redraw();
    }
    m_cancelSelection->setEnabled( false );
}

void DateBar::DateBarWidget::emitRangeSelection( const DB::ImageDate&  range )
{
    emit dateRangeChange( range );
}

int DateBar::DateBarWidget::unitForDate( const QDateTime& date ) const
{
    for ( int unit = 0; unit < numberOfUnits(); ++unit ) {
        if ( m_currentHandler->date( unit ) <= date && date < m_currentHandler->date( unit +1 ) )
            return unit;
    }
    return -1;
}

void DateBar::DateBarWidget::emitDateSelected()
{
    emit dateSelected( currentDateRange(), includeFuzzyCounts() );
}

void DateBar::DateBarWidget::wheelEvent( QWheelEvent * e )
{
    if ( e->modifiers() & Qt::ControlModifier ) {
        if ( e->delta() > 0 )
            zoomIn();
        else
            zoomOut();
        return;
    }
    int scrollAmount = e->delta() > 0 ? SCROLL_AMOUNT : -SCROLL_AMOUNT;
    if ( e->modifiers() & Qt::ShiftModifier )
        scrollAmount *= SCROLL_ACCELERATION;
    scroll( scrollAmount );
}

// vi:expandtab:tabstop=4 shiftwidth=4:
