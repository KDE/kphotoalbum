#include "MouseHandler.h"
#include "DateBar.h"
#include <math.h>
#include <qtimer.h>
#include <qcursor.h>

/**
 * \class DateBar::MouseHandler
 * \brief Base class for handling mouse events in the \ref DateBar
 *
 * The mouse events in the date bar are handled by subclasses of MouseHandler.
 * The subclasses are:
 * \li DateBar::BarDragHandler - used during dragging the bar (control left mouse button)
 * \li DateBar::FocusItemDragHandler - used during dragging the focus item (drag the top of the bar)
 * \li DateBar::SelectionHandler - used during range selection (drag on the bottom of the bar)
 */

/**
 * \class DateBar::BarDragHandler
 * \brief Mouse handler used when dragging the date bar (using control left mouse button on the bar)
 */

/**
 * \class DateBar::FocusItemDragHandler
 * \brief Handler used during dragging of the focus rectangle in the date bar (mouse button on upper part of the bar)
 */

/**
 * \class DateBar::SelectionHandler
 * \brief Handler used during range selection in the date bar (mouse button on lower part of the bar)
 */
DateBar::MouseHandler::MouseHandler( DateBar* dateBar )
    :QObject( dateBar, "handler" ), _dateBar( dateBar )
{
    _autoScrollTimer = new QTimer( this );
    connect( _autoScrollTimer, SIGNAL( timeout() ), this, SLOT( autoScroll() ) );
}

void DateBar::MouseHandler::autoScroll()
{
    mouseMoveEvent( _dateBar->mapFromGlobal( QCursor::pos() ).x() );
}

void DateBar::MouseHandler::startAutoScroll()
{
    _autoScrollTimer->start( 100 );
}

void DateBar::MouseHandler::endAutoScroll()
{
    _autoScrollTimer->stop();
}

DateBar::SelectionHandler::SelectionHandler( DateBar* dateBar )
    :MouseHandler( dateBar )
{
}

void DateBar::SelectionHandler::mousePressEvent( int x )
{
    int unit = _dateBar->unitAtPos( x );
    _start = _dateBar->dateForUnit( unit );
    _end = _dateBar->dateForUnit( unit + 1 );
}

void DateBar::SelectionHandler::mouseMoveEvent( int x )
{
    int unit = _dateBar->unitAtPos( x );
    QDateTime date = _dateBar->dateForUnit( unit );
    if ( _start < date )
        _end = _dateBar->dateForUnit( unit + 1 );
    else
        _end = date;
    _dateBar->redraw();
}




DateBar::FocusItemDragHandler::FocusItemDragHandler( DateBar* dateBar )
    : MouseHandler( dateBar )
{
}

void DateBar::FocusItemDragHandler::mousePressEvent( int x )
{
    _dateBar->_currentUnit = _dateBar->unitAtPos( x );
    _dateBar->_currentDate = _dateBar->dateForUnit( _dateBar->_currentUnit );
    if ( _dateBar->hasSelection() && ! _dateBar->currentSelection().includes( _dateBar->_currentDate ) )
        _dateBar->clearSelection();
}

void DateBar::FocusItemDragHandler::mouseMoveEvent( int x )
{
    int oldUnit = _dateBar->_currentUnit;
    int newUnit = ( x - _dateBar->barAreaGeometry().left() )/_dateBar->_barWidth;

    // Don't scroll further down than the last image
    // We use oldUnit here, to ensure that we scroll all the way to the end
    // better scroll a bit over than not all the way.
    if ( (newUnit > oldUnit &&
          _dateBar->dateForUnit( oldUnit ) > _dateBar->_dates->upperLimit() ) ||
         ( newUnit < oldUnit &&
           _dateBar->dateForUnit( oldUnit ) < _dateBar->_dates->lowerLimit() ) )
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





DateBar::BarDragHandler::BarDragHandler( DateBar* dateBar )
    : MouseHandler( dateBar )
{
}

void DateBar::BarDragHandler::mousePressEvent( int x )
{
    _movementOffset = _dateBar->_currentUnit * _dateBar->_barWidth - ( x - _dateBar->barAreaGeometry().left() );

}

void DateBar::BarDragHandler::mouseMoveEvent( int x )
{
    int oldUnit = _dateBar->_currentUnit;
    int newUnit = ( x + _movementOffset - _dateBar->barAreaGeometry().left() )/_dateBar->_barWidth;

    // Don't scroll further down than the last image
    // We use oldUnit here, to ensure that we scroll all the way to the end
    // better scroll a bit over than not all the way.
    if ( (newUnit > oldUnit &&
            _dateBar->dateForUnit( 0 ) < _dateBar->_dates->lowerLimit() ) ||
           ( newUnit < oldUnit &&
             _dateBar->dateForUnit( _dateBar->numberOfUnits() ) > _dateBar->_dates->upperLimit() ) )
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

QDateTime DateBar::SelectionHandler::min() const
{
    if ( _start < _end )
        return _start;
    else
        return _end;
}

QDateTime DateBar::SelectionHandler::max() const
{
    if ( _start >= _end )
        return _dateBar->dateForUnit( 1,_start );
    else
        return _end;
}

void DateBar::SelectionHandler::clearSelection()
{
    _start = QDateTime();
    _end = QDateTime();
}

void DateBar::SelectionHandler::mouseReleaseEvent()
{
    _dateBar->emitRangeSelection( dateRange() );
}

ImageDate DateBar::SelectionHandler::dateRange() const
{
    return ImageDate( min(), max() );
}

bool DateBar::SelectionHandler::hasSelection() const
{
    return min().isValid();
}


#include "MouseHandler.moc"
