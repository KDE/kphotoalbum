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

#ifndef DATEBAR_H
#define DATEBAR_H
#include <qwidget.h>
#include <qpixmap.h>
#include <qdatetime.h>
#include "imagedaterange.h"
#include "imagedaterangecollection.h"
#include "dateviewhandler.h"
class QPopupMenu;
class QToolButton;

class DateBar :public QWidget {
    Q_OBJECT

public:
    DateBar( QWidget* parent, const char* name = 0 );
    enum ViewType { DecadeView, YearView, MonthView, WeekView, DayView, HourView };
    bool includeFuzzyCounts() const;

public slots:
    void setViewType( ViewType tp );
    void setDate( const QDateTime& date );
    void setImageRangeCollection( const ImageDateRangeCollection& );
    void scrollLeft();
    void scrollRight();
    void scroll( int units );
    void zoomIn();
    void zoomOut();
    void setBarWidth( int width );
    void setIncludeFuzzyCounts( bool );
    void setShowResolutionIndicator( bool );

signals:
    void canZoomIn( bool );
    void canZoomOut( bool );
    void dateSelected( const ImageDateRange&, bool includeRanges );
    void toolTipInfo( const QString& );

public:
    // Overriden methods for internal purpose
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;
    virtual void paintEvent( QPaintEvent* event );
    virtual void resizeEvent( QResizeEvent* event );
    virtual void mousePressEvent( QMouseEvent* event );
    virtual void mouseMoveEvent( QMouseEvent* event );
    virtual void mouseReleaseEvent( QMouseEvent* event );

protected:
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
    virtual void contextMenuEvent( QContextMenuEvent* );
    void drawArrow( QPainter&, const QPoint& start, const QPoint& end );
    void startAutoScroll();
    void endAutoScroll();
    void doScroll( int x );
    void updateArrowState();
    ImageDateRange currentDateRange() const;
    void showStatusBarTip( const QPoint& pos );
    ImageDateRange rangeAt( const QPoint& );

protected slots:
    void autoScroll();

private:
    QPixmap _buffer;
    friend class DateBarTip;

    ImageDateRangeCollection _dates;
    DecadeViewHandler _decadeViewHandler;
    YearViewHandler _yearViewHandler;
    MonthViewHandler _monthViewHandler;
    WeekViewHandler _weekViewHandler;
    DayViewHandler _dayViewHandler;
    HourViewHandler _hourViewHandler;
    DateViewHandler* _currentHandler;
    ViewType _tp;

    QToolButton* _rightArrow;
    QToolButton* _leftArrow;
    QToolButton* _zoomIn;
    QToolButton* _zoomOut;

    int _currentUnit;
    QDateTime _currentDate;
    bool _movingBar;
    int _movementOffset;
    int _barWidth;
    bool _includeFuzzyCounts;
    QPopupMenu* _contextMenu;
    bool _showResolutionIndicator;
    QTimer* _autoScrollTimer;
};


#endif /* DATEBAR_H */

