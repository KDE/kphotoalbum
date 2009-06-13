#include "CenteringIconView.h"
#include <QTimer>
#include <QApplication>
#include <cmath>

const int CELLWIDTH = 200;
const int CELLHEIGHT = 150;

Browser::CenteringIconView::CenteringIconView( QWidget* parent )
    : QListView( parent ), _viewMode( NormalIconView )
{
    setGridSize( QSize(CELLWIDTH, CELLHEIGHT) );
    setViewportMargins ( 200,150,200,150 );

    viewport()->setAutoFillBackground(false);

    QListView::setViewMode( QListView::IconMode );

    _resizeTimer = new QTimer( this );
    _resizeTimer->setSingleShot( true );
    connect( _resizeTimer, SIGNAL( timeout() ), this, SLOT( setupMargin() ) );
}

void Browser::CenteringIconView::setViewMode( ViewMode viewMode )
{
    _viewMode = viewMode;
    if ( viewMode == CenterView ) {
        setGridSize( QSize(200,150) );
        setupMargin();
    }
    else {
        setGridSize( QSize() );
        setViewportMargins ( 0,0,0,0 );
    }
}

void Browser::CenteringIconView::setupMargin()
{
    if ( _viewMode == NormalIconView || !model() || !viewport())
        return;

    // In this code I'll call resize, which calls resizeEvent, which calls
    // this code. So I need to break that loop, which I do here.
    static bool inAction = false;
    if ( inAction )
        return;
    inAction = true;

    const int count = model()->rowCount();
    int columns = std::sqrt(count);

    // The 40 and 10 below are some magic numbers. I think the reason I
    // need them is that the viewport doesn't cover all of the list views area.
    const int w = width() -40;
    const int h = height() -10;

    columns = qMax(2, qMin( columns, w/CELLWIDTH ) );

    const int xMargin = (w-columns*CELLWIDTH)/2;
    const int rows = std::ceil(count/columns);
    const int yMargin = qMax( 0, (int) (h-rows*CELLHEIGHT )/2);

    setViewportMargins ( xMargin, yMargin, xMargin, yMargin );
    inAction = false;
}

void Browser::CenteringIconView::resizeEvent( QResizeEvent* event )
{
    QListView::resizeEvent( event );
    // Without this timer, the view will flicker a lot during resize.
    _resizeTimer->start( 100 );
}

void Browser::CenteringIconView::setModel( QAbstractItemModel* model )
{
    QListView::setModel( model );
    setupMargin();
}

void Browser::CenteringIconView::showEvent( QShowEvent* event )
{
    setupMargin();
    QListView::showEvent(event);
}

