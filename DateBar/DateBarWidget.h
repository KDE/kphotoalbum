/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef DATEBAR_H
#define DATEBAR_H
#include <QDateTime>
#include <QExplicitlySharedDataPointer>
#include <QPixmap>
#include <QWidget>

#include <DateBar/ViewHandler.h>

class QMenu;
class QKeyEvent;
class QMouseEvent;
class QFocusEvent;
class QResizeEvent;
class QPaintEvent;
class QWheelEvent;
class QContextMenuEvent;
class QToolButton;

namespace DB { class ImageDateCollection; class ImageDate; }

namespace DateBar
{
class MouseHandler;
class FocusItemDragHandler;
class BarDragHandler;
class SelectionHandler;

class DateBarWidget :public QWidget {
    Q_OBJECT

public:
    explicit DateBarWidget( QWidget* parent );
    enum ViewType { DecadeView, YearView, MonthView, WeekView, DayView, HourView, MinuteView };
    bool includeFuzzyCounts() const;

public slots:
    void clearSelection();
    void setViewType( ViewType tp, bool redrawNow=true );
    void setDate( const QDateTime& date );
    void setImageDateCollection(const QExplicitlySharedDataPointer<DB::ImageDateCollection> & );
    void scrollLeft();
    void scrollRight();
    void scroll( int units );
    void zoomIn();
    void zoomOut();
    void setHistogramBarSize( const QSize& size );
    void setIncludeFuzzyCounts( bool );
    void setShowResolutionIndicator( bool );
    void setAutomaticRangeAdjustment( bool );

signals:
    void canZoomIn( bool );
    void canZoomOut( bool );
    void dateSelected( const DB::ImageDate&, bool includeRanges );
    void toolTipInfo( const QString& );
    void dateRangeChange( const DB::ImageDate& );
    void dateRangeCleared();

public:
    // Overridden methods for internal purpose
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
    DB::ImageDate rangeForUnit( int unit );
    void placeAndSizeButtons();
    int unitAtPos( int x ) const;
    QDateTime dateForUnit( int unit, const QDateTime& offset = QDateTime() ) const;
    int unitForDate( const QDateTime& date ) const;
    bool isUnitSelected( int unit ) const;
    bool hasSelection() const;
    DB::ImageDate currentSelection() const;
    void emitDateSelected();
    void emitRangeSelection( const DB::ImageDate& );

private:
    void setViewHandlerForType( ViewType tp );
    QPixmap m_buffer;
    friend class DateBarTip;

    QExplicitlySharedDataPointer<DB::ImageDateCollection> m_dates;
    DecadeViewHandler m_decadeViewHandler;
    YearViewHandler m_yearViewHandler;
    MonthViewHandler m_monthViewHandler;
    WeekViewHandler m_weekViewHandler;
    DayViewHandler m_dayViewHandler;
    HourViewHandler m_hourViewHandler;
    MinuteViewHandler m_minuteViewHandler;
    ViewHandler* m_currentHandler;
    ViewType m_tp;

    MouseHandler* m_currentMouseHandler;
    FocusItemDragHandler* m_focusItemDragHandler;
    BarDragHandler* m_barDragHandler;
    SelectionHandler* m_selectionHandler;
    friend class Handler;
    friend class FocusItemDragHandler;
    friend class BarDragHandler;
    friend class SelectionHandler;

    QToolButton* m_rightArrow;
    QToolButton* m_leftArrow;
    QToolButton* m_zoomIn;
    QToolButton* m_zoomOut;
    QToolButton* m_cancelSelection;

    int m_currentUnit;
    QDateTime m_currentDate;
    int m_barWidth;
    int m_barHeight;
    bool m_includeFuzzyCounts;
    QMenu* m_contextMenu;
    bool m_showResolutionIndicator;
    bool m_doAutomaticRangeAdjustment;
};

}

#endif /* DATEBAR_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
