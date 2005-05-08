#include "datebarmousehandler.h"
#include "datebar.h"
#include <math.h>
#include <qtimer.h>
#include <qcursor.h>

DateBarMouseHandler::Handler::Handler( DateBar* dateBar )
    :QObject( dateBar, "handler" ), _dateBar( dateBar )
{
    _autoScrollTimer = new QTimer( this );
    connect( _autoScrollTimer, SIGNAL( timeout() ), this, SLOT( autoScroll() ) );
}

void DateBarMouseHandler::Handler::autoScroll()
{
    mouseMoveEvent( _dateBar->mapFromGlobal( QCursor::pos() ).x() );
}

void DateBarMouseHandler::Handler::startAutoScroll()
{
    _autoScrollTimer->start( 100 );
}

void DateBarMouseHandler::Handler::endAutoScroll()
{
    _autoScrollTimer->stop();
}

DateBarMouseHandler::Selection::Selection( DateBar* dateBar )
    :Handler( dateBar )
{
}

void DateBarMouseHandler::Selection::mousePressEvent( int x )
{
    int unit = _dateBar->unitAtPos( x );
    _start = _dateBar->dateForUnit( unit );
    _end = _dateBar->dateForUnit( unit + 1 );
}

void DateBarMouseHandler::Selection::mouseMoveEvent( int x )
{
    int unit = _dateBar->unitAtPos( x );
    QDateTime date = _dateBar->dateForUnit( unit );
    if ( _start < date )
        _end = _dateBar->dateForUnit( unit + 1 );
    else
        _end = date;
    _dateBar->redraw();
}




DateBarMouseHandler::FocusItem::FocusItem( DateBar* dateBar )
    : Handler( dateBar )
{
}

void DateBarMouseHandler::FocusItem::mousePressEvent( int x )
{
    _dateBar->_currentUnit = _dateBar->unitAtPos( x );
    _dateBar->_currentDate = _dateBar->dateForUnit( _dateBar->_currentUnit );
    if ( _dateBar->hasSelection() && ! _dateBar->currentSelection().includes( _dateBar->_currentDate ) )
        _dateBar->clearSelection();
}

void DateBarMouseHandler::FocusItem::mouseMoveEvent( int x )
{
    int oldUnit = _dateBar->_currentUnit;
    int newUnit = ( x - _dateBar->barAreaGeometry().left() )/_dateBar->_barWidth;

    // Don't scroll further down than the last image
    // We use oldUnit here, to ensure that we scroll all the way to the end
    // better scroll a bit over than not all the way.
    if ( (newUnit > oldUnit &&
          _dateBar->dateForUnit( oldUnit ) > _dateBar->_dates.upperLimit() ) ||
         ( newUnit < oldUnit &&
           _dateBar->dateForUnit( oldUnit ) < _dateBar->_dates.lowerLimit() ) )
        return;
    _dateBar->_currentUnit = newUnit;

    static double rest = 0;
    if ( _dateBar->_currentUnit < 0 || _dateBar->_currentUnit > _dateBar->numberOfUnits() ) {
        // Slow down scrolling outside date bar.
        double newUnit = oldUnit + ( _dateBar->_currentUnit - oldUnit ) / 4.0 + rest;
        _dateBar->_currentUnit = (int) floor( newUnit );
        rest = newUnit - _dateBar->_currentUnit;
        startAutoScroll();
    }

    _dateBar->_currentDate = _dateBar->dateForUnit( _dateBar->_currentUnit );
    _dateBar->_currentUnit = QMAX( _dateBar->_currentUnit, 0 );
    _dateBar->_currentUnit = QMIN( _dateBar->_currentUnit, _dateBar->numberOfUnits() );
    _dateBar->redraw();
    _dateBar->emitDateSelected();
}





DateBarMouseHandler::DateArea::DateArea( DateBar* dateBar )
    : Handler( dateBar )
{
}

void DateBarMouseHandler::DateArea::mousePressEvent( int x )
{
    _movementOffset = _dateBar->_currentUnit * _dateBar->_barWidth - ( x - _dateBar->barAreaGeometry().left() );

}

void DateBarMouseHandler::DateArea::mouseMoveEvent( int x )
{
    int oldUnit = _dateBar->_currentUnit;
    int newUnit = ( x + _movementOffset - _dateBar->barAreaGeometry().left() )/_dateBar->_barWidth;

    // Don't scroll further down than the last image
    // We use oldUnit here, to ensure that we scroll all the way to the end
    // better scroll a bit over than not all the way.
    if ( (newUnit > oldUnit &&
            _dateBar->dateForUnit( 0 ) < _dateBar->_dates.lowerLimit() ) ||
           ( newUnit < oldUnit &&
             _dateBar->dateForUnit( _dateBar->numberOfUnits() ) > _dateBar->_dates.upperLimit() ) )
        return;

    _dateBar->_currentUnit = newUnit;

    if ( _dateBar->_currentUnit < 0 ) {
        _dateBar->_currentDate = _dateBar->dateForUnit( - _dateBar->_currentUnit );
        _dateBar->_currentUnit = 0;
        _movementOffset = _dateBar->barAreaGeometry().left() - x;
    }
    else if ( _dateBar->_currentUnit > _dateBar->numberOfUnits() ) {
        int diff = _dateBar->numberOfUnits() - _dateBar->_currentUnit;
        _dateBar->_currentDate = _dateBar->dateForUnit( _dateBar->numberOfUnits()+diff );
        _dateBar->_currentUnit = _dateBar->numberOfUnits();
        _movementOffset = (_dateBar->numberOfUnits()*_dateBar->_barWidth ) - x + _dateBar->_barWidth/ 2  ;
    }
    _dateBar->redraw();
    _dateBar->emitDateSelected();
}

QDateTime DateBarMouseHandler::Selection::min() const
{
    if ( _start < _end )
        return _start;
    else
        return _end;
}

QDateTime DateBarMouseHandler::Selection::max() const
{
    if ( _start >= _end )
        return _dateBar->dateForUnit( 1,_start );
    else
        return _end;
}

void DateBarMouseHandler::Selection::clearSelection()
{
    _start = QDateTime();
    _end = QDateTime();
}

void DateBarMouseHandler::Selection::mouseReleaseEvent()
{
    _dateBar->emitRangeSelection( dateRange() );
}

ImageDateRange DateBarMouseHandler::Selection::dateRange() const
{
    return ImageDateRange( min(), max() );
}

bool DateBarMouseHandler::Selection::hasSelection() const
{
    return min().isValid();
}


#include "datebarmousehandler.moc"
