// SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DATEBAR_H
#define DATEBAR_H
#include "ViewHandler.h"

#include <Utilities/FastDateTime.h>
#include <QExplicitlySharedDataPointer>
#include <QPixmap>
#include <QWidget>

class KActionCollection;
class QMenu;
class QKeyEvent;
class QMouseEvent;
class QFocusEvent;
class QResizeEvent;
class QPaintEvent;
class QWheelEvent;
class QContextMenuEvent;
class QToolButton;
class QFontMetrics;

namespace DB
{
class ImageDateCollection;
class ImageDate;
}

namespace DateBar
{
class MouseHandler;
class FocusItemDragHandler;
class BarDragHandler;
class SelectionHandler;

/**
 * @brief The DateBarWidget class provides a histogram-like depiction of the image distribution over time.
 * If \ref includeFuzzyCounts() is \c true,
 * then both exact and fuzzy dates are taken into account and shown in different style (currently yellow and green).
 * If enough space is available, the number of images within each time period is printed inside each box.
 *
 * ## "unit" concept
 * A central concept in the widget design is that of a time \c unit.
 * Units are an integer offset into the number of available "boxes".
 * The number of units (or boxes) is calculated according to the available space (see \ref numberOfUnits()).
 * Each unit corresponds to a time period at the current resolution and offset.
 *
 * The time resulution is represented by the \c ViewHandler, and the offset is stored in \c m_currentDate.
 *
 */
class DateBarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DateBarWidget(QWidget *parent);
    enum ViewType { DecadeView,
                    YearView,
                    MonthView,
                    WeekView,
                    DayView,
                    HourView,
                    TenMinuteView,
                    MinuteView };
    /**
     * @brief includeFuzzyCounts
     * @return \c true if date ranges are shown, \c false otherwise.
     * @see setIncludeFuzzyCounts
     */
    bool includeFuzzyCounts() const;
    KActionCollection *actions();

public slots:
    void clearSelection();
    void setViewType(ViewType tp, bool redrawNow = true);
    void setDate(const Utilities::FastDateTime &date);
    void setImageDateCollection(const QExplicitlySharedDataPointer<DB::ImageDateCollection> &);
    void scrollLeft();
    void scrollRight();
    void scroll(int units);
    void zoomIn();
    void zoomOut();
    void setHistogramBarSize(const QSize &size);
    void setIncludeFuzzyCounts(bool);
    /**
     * @brief setShowResolutionIndicator
     * If set to \c true, an indicator is shown to indicate the current ViewType.
     * The indicator indicates the size of one unit / histogram box alongside a text
     * indicating the respective temporal resolution (e.g. "10 minutes").
     */
    void setShowResolutionIndicator(bool);
    /**
     * @brief setAutomaticRangeAdjustment
     * If set to \c true, the ViewType and range is adjusted automatically to best
     * match the current set of images.
     */
    void setAutomaticRangeAdjustment(bool);

signals:
    void canZoomIn(bool);
    void canZoomOut(bool);
    void dateSelected(const DB::ImageDate &, bool includeRanges);
    void toolTipInfo(const QString &);
    void dateRangeChange(const DB::ImageDate &);
    void dateRangeCleared();
    void dateRangeSelected(bool);

public:
    // Overridden methods for internal purpose
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *) override;
    void focusOutEvent(QFocusEvent *) override;
    void wheelEvent(QWheelEvent *e) override;

    /**
     * @brief redraw the widget
     * This method creates a QPainter and then uses the draw* methods to draw the different parts of the widget.
     * \see drawTickMarks
     * \see drawHistograms
     * \see drawFocusRectangle
     * \see drawResolutionIndicator
     */
    void redraw();
    void drawTickMarks(QPainter &p, const QRect &textRect);
    void drawHistograms(QPainter &p);
    void drawFocusRectangle(QPainter &p);
    void drawResolutionIndicator(QPainter &p, int *leftEdge);
    /**
     * @brief zoom in or out by a number of steps.
     * One steps corresponds to one step in the ViewType.
     * @param steps positive steps to increase temporal resolution, negative to decrease.
     */
    void zoom(int steps);
    QRect barAreaGeometry() const;
    QRect tickMarkGeometry() const;
    QRect dateAreaGeometry() const;
    int numberOfUnits() const;
    void drawArrow(QPainter &, const QPoint &start, const QPoint &end);
    void updateArrowState();
    DB::ImageDate currentDateRange() const;
    void showStatusBarTip(const QPoint &pos);
    DB::ImageDate rangeAt(const QPoint &);
    DB::ImageDate rangeForUnit(int unit);
    void placeAndSizeButtons();
    /**
     * @brief unitAtPos maps horizontal screen coordinates to units.
     * @param x a valid pixel offset in the histogram area
     * @return a unit index between 0 and numberOfUnits
     */
    int unitAtPos(int x) const;
    Utilities::FastDateTime dateForUnit(int unit, const Utilities::FastDateTime &offset = Utilities::FastDateTime()) const;
    /**
     * @brief unitForDate return the unit index corresponding to the date/time.
     * @param date a valid Utilities::FastDateTime.
     * @return An integer greater or equal to 0 if \p date is in view, -1 otherwise.
     */
    int unitForDate(const Utilities::FastDateTime &date) const;
    bool isUnitSelected(int unit) const;
    bool hasSelection() const;
    DB::ImageDate currentSelection() const;
    void emitDateSelected();
    void emitRangeSelection(const DB::ImageDate &);

private:
    void setViewHandlerForType(ViewType tp);
    int stringWidth(const QFontMetrics &fontMetrics, const QString &text) const;
    QPixmap m_buffer;
    friend class DateBarTip;

    QExplicitlySharedDataPointer<DB::ImageDateCollection> m_dates;
    DecadeViewHandler m_decadeViewHandler;
    YearViewHandler m_yearViewHandler;
    MonthViewHandler m_monthViewHandler;
    WeekViewHandler m_weekViewHandler;
    DayViewHandler m_dayViewHandler;
    HourViewHandler m_hourViewHandler;
    TenMinuteViewHandler m_tenMinuteViewHandler;
    MinuteViewHandler m_minuteViewHandler;
    ViewHandler *m_currentHandler;
    ViewType m_tp;

    MouseHandler *m_currentMouseHandler;
    FocusItemDragHandler *m_focusItemDragHandler;
    BarDragHandler *m_barDragHandler;
    SelectionHandler *m_selectionHandler;
    friend class Handler;
    friend class FocusItemDragHandler;
    friend class BarDragHandler;
    friend class SelectionHandler;

    QToolButton *m_rightArrow;
    QToolButton *m_leftArrow;
    QToolButton *m_zoomIn;
    QToolButton *m_zoomOut;
    QToolButton *m_cancelSelection;

    int m_currentUnit;
    Utilities::FastDateTime m_currentDate;
    int m_barWidth; ///< width of a single unit in pixel
    int m_barHeight;
    bool m_includeFuzzyCounts;
    QMenu *m_contextMenu;
    bool m_showResolutionIndicator;
    bool m_doAutomaticRangeAdjustment;
    KActionCollection *m_actionCollection;
};
}

#endif /* DATEBAR_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
