#include "CenteringIconView.h"
#include <Utilities/BooleanGuard.h>
#include <cmath>

const int CELLWIDTH = 200;
const int CELLHEIGHT = 150;

Browser::CenteringIconView::CenteringIconView( QWidget* parent )
    : QListView( parent ), _viewMode( NormalIconView )
{
    setGridSize( QSize(CELLWIDTH, CELLHEIGHT) );
    viewport()->setAutoFillBackground(false);

    QListView::setViewMode( QListView::IconMode );
}

void Browser::CenteringIconView::setViewMode( ViewMode viewMode )
{
    _viewMode = viewMode;
    if ( viewMode == CenterView ) {
        setGridSize( QSize(200,150) );
        setupMargins();
    }
    else {
        setGridSize( QSize() );
        setViewportMargins ( 0,0,0,0 );
    }
}

void Browser::CenteringIconView::setupMargins()
{
    if ( _viewMode == NormalIconView || !model() || !viewport())
        return;

    // In this code I'll call resize, which calls resizeEvent, which calls
    // this code. So I need to break that loop, which I do here.
    static bool inAction = false;
    Utilities::BooleanGuard guard( inAction );
    if ( !guard.canContinue() )
        return;

    const int count = model()->rowCount();
    if ( count == 0 )
        return;

    const int columns = columnCount( count );
    const int rows = std::ceil(1.0*count/columns);

    const int xMargin = (availableWidth()-columns*CELLWIDTH)/2;
    const int yMargin = qMax( 0, (int) (availableHeight()-rows*CELLHEIGHT )/2);

    setViewportMargins ( xMargin, yMargin, xMargin, yMargin );
}

void Browser::CenteringIconView::resizeEvent( QResizeEvent* event )
{
    QListView::resizeEvent( event );
    setupMargins();
}

void Browser::CenteringIconView::setModel( QAbstractItemModel* model )
{
    QListView::setModel( model );
    setupMargins();
}

void Browser::CenteringIconView::showEvent( QShowEvent* event )
{
    setupMargins();
    QListView::showEvent(event);
}

int Browser::CenteringIconView::columnCount( int elementCount ) const
{
    const int preferredCount = std::ceil( std::sqrt( elementCount ) );
    const int maxVisibleColumnsPossible = availableWidth()/CELLWIDTH;
    const int maxVisibleRowsPossible = qMax(1, availableHeight()/CELLHEIGHT);
    const int colCountToMakeAllRowVisible = std::ceil( 1.0*elementCount / maxVisibleRowsPossible);

    int res = preferredCount;
    res = qMax( res, colCountToMakeAllRowVisible ); // Should we go for more due to limited row count?
    res = qMin( res, maxVisibleColumnsPossible ); // No more than maximal visible count
    res = qMax( res, 1 ); // at least 1
    return res;
}

int Browser::CenteringIconView::availableWidth() const
{
    // The 40 and 10 below are some magic numbers. I think the reason I
    // need them is that the viewport doesn't cover all of the list views area.
    return width() - 40;
}

int Browser::CenteringIconView::availableHeight() const
{
    return height()-10;
}

