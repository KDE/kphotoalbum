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

#ifndef DATEBAR_H
#define DATEBAR_H
#include <qwidget.h>
#include <qpixmap.h>
#include <qdatetime.h>
#include "DB/ImageDateCollection.h"
#include "DateBar/ViewHandler.h"
#include "DateBar/MouseHandler.h"
#include <ksharedptr.h>
class QPopupMenu;
class QToolButton;

namespace DateBar
{

class DateBarWidget :public QWidget {
    Q_OBJECT

public:
    DateBarWidget( QWidget* parent, const char* name = 0 );
    enum ViewType { DecadeView, YearView, MonthView, WeekView, DayView, HourView };
    bool includeFuzzyCounts() const;

public slots:
    void setViewType( ViewType tp );
    void setDate( const QDateTime& date );
    void setImageDateCollection( const KSharedPtr<DB::ImageDateCollection>& );
    void scrollLeft();
    void scrollRight();
    void scroll( int units );
    void zoomIn();
    void zoomOut();
    void setHistogramBarSize( const QSize& size );
    void setIncludeFuzzyCounts( bool );
    void setShowResolutionIndicator( bool );

signals:
    void canZoomIn( bool );
    void canZoomOut( bool );
    void dateSelected( const DB::ImageDate&, bool includeRanges );
    void toolTipInfo( const QString& );
    void dateRangeChange( const DB::ImageDate& );
    void dateRangeCleared();

public:
    // Overriden methods for internal purpose
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

protected:
    virtual void paintEvent( QPaintEvent* event );
    virtual void resizeEvent( QResizeEvent* event );
    virtual void mousePressEvent( QMouseEvent* event );
    virtual void mouseMoveEvent( QMouseEvent* event );
    virtual void mouseReleaseEvent( QMouseEvent* event );
    virtual void contextMenuEvent( QContextMenuEvent* );
    virtual void keyPressEvent( QKeyEvent* event );
    virtual void focusInEvent( QFocusEvent* );
    virtual void focusOutEvent( QFocusEvent* );
    virtual void wheelEvent( QWheelEvent * e );

    void redraw();
    void drawTickMarks( QPainter& p, const QRect& textRect );
    void drawHistograms( QPainter& p );
    void drawFocusRectagle( QPainter& p );
    void drawResolutionIndicator( QPainter& p, int* leftEdge );
    void zoom( int );
    QRect barAreaGeometry() const;
    QRect tickMarkGeometry() const;
    QRect dateAreaGeometry() const;
    int numberOfUnits() const;
    void drawArrow( QPainter&, const QPoint& start, const QPoint& end );
    void updateArrowState();
    DB::ImageDate currentDateRange() const;
    void showStatusBarTip( const QPoint& pos );
    DB::ImageDate rangeAt( const QPoint& );
    void placeAndSizeButtons();
    int unitAtPos( int x ) const;
    QDateTime dateForUnit( int unit, const QDateTime& offset = QDateTime() ) const;
    int unitForDate( const QDateTime& date ) const;
    bool isUnitSelected( int unit ) const;
    bool hasSelection() const;
    DB::ImageDate currentSelection() const;
    void emitDateSelected();
    void emitRangeSelection( const DB::ImageDate& );

protected slots:
    void clearSelection();

private:
    QPixmap _buffer;
    friend class DateBarTip;

    KSharedPtr<DB::ImageDateCollection> _dates;
    DecadeViewHandler _decadeViewHandler;
    YearViewHandler _yearViewHandler;
    MonthViewHandler _monthViewHandler;
    WeekViewHandler _weekViewHandler;
    DayViewHandler _dayViewHandler;
    HourViewHandler _hourViewHandler;
    ViewHandler* _currentHandler;
    ViewType _tp;

    MouseHandler* _currentMouseHandler;
    FocusItemDragHandler* _focusItemDragHandler;
    BarDragHandler* _barDragHandler;
    SelectionHandler* _selectionHandler;
    friend class Handler;
    friend class FocusItemDragHandler;
    friend class BarDragHandler;
    friend class SelectionHandler;

    QToolButton* _rightArrow;
    QToolButton* _leftArrow;
    QToolButton* _zoomIn;
    QToolButton* _zoomOut;
    QToolButton* _cancelSelection;

    int _currentUnit;
    QDateTime _currentDate;
    int _barWidth;
    int _barHeight;
    bool _includeFuzzyCounts;
    QPopupMenu* _contextMenu;
    bool _showResolutionIndicator;
};

}

#endif /* DATEBAR_H */

