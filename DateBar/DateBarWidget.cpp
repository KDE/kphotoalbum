// SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2021-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2024 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "DateBarWidget.h"

#include "MouseHandler.h"

#include <DB/ImageDateCollection.h>
#include <DB/ImageInfoList.h>
#include <kpabase/SettingsData.h>

#include <Utilities/FastDateTime.h>
#include <KActionCollection>
#include <KLocalizedString>
#include <QAction>
#include <QContextMenuEvent>
#include <QDebug>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QIcon>
#include <QLocale>
#include <QMenu>
#include <QPainter>
#include <QToolButton>
#include <math.h>

namespace
{
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

DateBar::DateBarWidget::DateBarWidget(QWidget *parent)
    : QWidget(parent)
    , m_currentHandler(&m_yearViewHandler)
    , m_tp(YearView)
    , m_currentMouseHandler(nullptr)
    , m_currentUnit(0)
    , m_currentDate(Utilities::FastDateTime::currentDateTime())
    , m_includeFuzzyCounts(true)
    , m_contextMenu(nullptr)
    , m_showResolutionIndicator(true)
    , m_doAutomaticRangeAdjustment(true)
    , m_actionCollection(new KActionCollection(this))
{
    setAccessibleName(i18nc("Accessible name for the date bar widget", "Datebar"));
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_barWidth = Settings::SettingsData::instance()->histogramSize().width();
    m_barHeight = Settings::SettingsData::instance()->histogramSize().height();

    auto scrollRightAction = m_actionCollection->addAction(QString::fromLatin1("datebar-scroll-right"));
    scrollRightAction->setShortcutContext(Qt::ApplicationShortcut);
    scrollRightAction->setText(i18n("Scroll right"));
    scrollRightAction->setAutoRepeat(true);
    connect(scrollRightAction, &QAction::triggered, this, &DateBarWidget::scrollRight);
    m_rightArrow = new QToolButton(this);
    m_rightArrow->setDefaultAction(scrollRightAction);
    m_rightArrow->setArrowType(Qt::RightArrow);

    auto scrollLeftAction = m_actionCollection->addAction(QString::fromLatin1("datebar-scroll-left"));
    scrollLeftAction->setShortcutContext(Qt::ApplicationShortcut);
    scrollLeftAction->setText(i18n("Scroll left"));
    scrollLeftAction->setAutoRepeat(true);
    connect(scrollLeftAction, &QAction::triggered, this, &DateBarWidget::scrollLeft);
    m_leftArrow = new QToolButton(this);
    m_leftArrow->setDefaultAction(scrollLeftAction);
    m_leftArrow->setArrowType(Qt::LeftArrow);

    auto zoomInAction = m_actionCollection->addAction(QString::fromLatin1("datebar-zoom-in"));
    zoomInAction->setShortcutContext(Qt::ApplicationShortcut);
    zoomInAction->setText(i18n("Zoom in"));
    zoomInAction->setIcon(QIcon::fromTheme(QStringLiteral("zoom-in")));
    zoomInAction->setEnabled(canZoomIn());
    connect(zoomInAction, &QAction::triggered, this, &DateBarWidget::zoomIn);
    connect(this, &DateBarWidget::zoomInEnabled, zoomInAction, &QAction::setEnabled);
    m_zoomIn = new QToolButton(this);
    m_zoomIn->setDefaultAction(zoomInAction);
    m_zoomIn->setFocusPolicy(Qt::ClickFocus);

    auto zoomOutAction = m_actionCollection->addAction(QString::fromLatin1("datebar-zoom-out"));
    zoomOutAction->setShortcutContext(Qt::ApplicationShortcut);
    zoomOutAction->setText(i18n("Zoom out"));
    zoomOutAction->setIcon(QIcon::fromTheme(QStringLiteral("zoom-out")));
    zoomOutAction->setEnabled(canZoomOut());
    connect(zoomOutAction, &QAction::triggered, this, &DateBarWidget::zoomOut);
    connect(this, &DateBarWidget::zoomOutEnabled, zoomOutAction, &QAction::setEnabled);
    m_zoomOut = new QToolButton(this);
    m_zoomOut->setDefaultAction(zoomOutAction);
    m_zoomOut->setFocusPolicy(Qt::ClickFocus);

    auto clearSelectionAction = m_actionCollection->addAction(QString::fromLatin1("datebar-clear-selection"));
    clearSelectionAction->setShortcutContext(Qt::ApplicationShortcut);
    clearSelectionAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-clear")));
    clearSelectionAction->setText(i18nc("The button clears the selection of a date range in the date bar.", "Clear date selection"));
    clearSelectionAction->setEnabled(false);
    connect(clearSelectionAction, &QAction::triggered, this, &DateBarWidget::clearSelection);
    connect(this, &DateBarWidget::dateRangeSelected, clearSelectionAction, &QAction::setEnabled);
    m_cancelSelection = new QToolButton(this);
    m_cancelSelection->setDefaultAction(clearSelectionAction);

    placeAndSizeButtons();

    m_focusItemDragHandler = new FocusItemDragHandler(this);
    m_barDragHandler = new BarDragHandler(this);
    m_selectionHandler = new SelectionHandler(this);

    setWhatsThis(xi18nc("@info", "<title>The date bar</title>"
                                 "<para><list>"
                                 "<item>Scroll using the arrow buttons, the scrollwheel, or the middle mouse button.</item>"
                                 "<item>Zoom using the +/- buttons or Ctrl + scrollwheel.</item>"
                                 "<item>Restrict the view to a date range selection: Click/drag below the timeline.</item>"
                                 "<item>Jump to a date by clicking on the histogram bar.</item>"
                                 "</list></para>"));
    setToolTip(whatsThis());

    connect(Settings::SettingsData::instance(), &Settings::SettingsData::histogramScaleChanged, this, &DateBarWidget::redraw);
    m_actionCollection->readSettings();
}

QSize DateBar::DateBarWidget::sizeHint() const
{
    int height = qMax(dateAreaGeometry().bottom() + BORDER_AROUND_WIDGET,
                      m_barHeight + BUTTON_WIDTH + 2 * BORDER_AROUND_WIDGET + 7);
    return QSize(800, height);
}

QSize DateBar::DateBarWidget::minimumSizeHint() const
{
    int height = qMax(dateAreaGeometry().bottom() + BORDER_AROUND_WIDGET,
                      m_barHeight + BUTTON_WIDTH + 2 * BORDER_AROUND_WIDGET + 7);
    return QSize(200, height);
}

bool DateBar::DateBarWidget::event(QEvent *event)
{
    if (event->type() == QEvent::PaletteChange) {
        QWidget::event(event);
        redraw();
        return true;
    }
    return QWidget::event(event);
}

void DateBar::DateBarWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, m_buffer);
}

void DateBar::DateBarWidget::redraw()
{
    if (m_buffer.isNull())
        return;

    QPainter p(&m_buffer);
    p.setRenderHint(QPainter::Antialiasing);
    p.setFont(font());

    // Fill with background pixels
    p.save();
    p.setPen(Qt::NoPen);
    p.setBrush(palette().brush(QPalette::Window));
    p.drawRect(rect());

    if (!m_dates) {
        p.restore();
        return;
    }

    // Draw the area with histograms
    QRect barArea = barAreaGeometry();

    p.setPen(palette().color(QPalette::Dark));
    p.setBrush(palette().brush(QPalette::Base));
    p.drawRect(barArea);
    p.restore();

    // shift the date bar by m_currentUnit units
    m_currentHandler->init(dateForUnit(-m_currentUnit, m_currentDate));

    int right;
    drawResolutionIndicator(p, &right);
    QRect rect = dateAreaGeometry();
    rect.setRight(right);
    rect.setLeft(rect.left() + BUTTON_WIDTH + 2);

    drawTickMarks(p, rect);
    drawHistograms(p);
    drawFocusRectangle(p);
    updateArrowState();
    repaint();
}

void DateBar::DateBarWidget::resizeEvent(QResizeEvent *event)
{
    placeAndSizeButtons();
    m_buffer = QPixmap(event->size());
    m_currentUnit = numberOfUnits() / 2;
    redraw();
}

void DateBar::DateBarWidget::drawTickMarks(QPainter &p, const QRect &textRect)
{
    QRect rect = tickMarkGeometry();
    p.save();
    p.setPen(QPen(palette().color(QPalette::Text), 1));

    QFont f(font());
    QFontMetrics fm(f);
    int fontHeight = fm.height();
    int unit = 0;
    QRect clip = rect;
    clip.setHeight(rect.height() + 2 + fontHeight);
    clip.setLeft(clip.left() + 2);
    clip.setRight(clip.right() - 2);
    p.setClipRect(clip);

    for (int x = rect.x(); x < rect.right(); x += m_barWidth, unit += 1) {
        // draw selection indication
        p.save();
        p.setPen(Qt::NoPen);
        p.setBrush(palette().brush(QPalette::Highlight));
        Utilities::FastDateTime date = dateForUnit(unit);
        if (isUnitSelected(unit))
            p.drawRect(QRect(x, rect.top(), m_barWidth, rect.height()));
        p.restore();

        // draw tickmarks
        int h = rect.height();
        if (m_currentHandler->isMajorUnit(unit)) {
            QString text = m_currentHandler->text(unit);
            int w = fm.horizontalAdvance(text);
            p.setFont(f);
            if (textRect.right() > x + w / 2 && textRect.left() < x - w / 2)
                p.drawText(x - w / 2, textRect.top(), w, fontHeight, Qt::TextSingleLine, text);
        } else if (m_currentHandler->isMidUnit(unit))
            h = (int)(2.0 / 3 * rect.height());
        else
            h = (int)(1.0 / 3 * rect.height());

        p.drawLine(x, rect.top(), x, rect.top() + h);
    }

    p.restore();
}

void DateBar::DateBarWidget::setViewType(ViewType tp, bool redrawNow)
{
    setViewHandlerForType(tp);
    if (hasSelection()) {
        centerDateRange(m_selectionHandler->min(), m_selectionHandler->max());
    }
    if (redrawNow)
        redraw();
    m_tp = tp;
}

void DateBar::DateBarWidget::setViewHandlerForType(ViewType tp)
{
    switch (tp) {
    case DecadeView:
        m_currentHandler = &m_decadeViewHandler;
        break;
    case YearView:
        m_currentHandler = &m_yearViewHandler;
        break;
    case MonthView:
        m_currentHandler = &m_monthViewHandler;
        break;
    case WeekView:
        m_currentHandler = &m_weekViewHandler;
        break;
    case DayView:
        m_currentHandler = &m_dayViewHandler;
        break;
    case HourView:
        m_currentHandler = &m_hourViewHandler;
        break;
    case TenMinuteView:
        m_currentHandler = &m_tenMinuteViewHandler;
        break;
    case MinuteView:
        m_currentHandler = &m_minuteViewHandler;
        break;
    }
}

void DateBar::DateBarWidget::setDate(const Utilities::FastDateTime &date)
{
    m_currentDate = date;
    if (hasSelection()) {
        if (currentSelection().start() > m_currentDate)
            m_currentDate = currentSelection().start();
        if (currentSelection().end() < m_currentDate)
            m_currentDate = currentSelection().end();
    }

    if (unitForDate(m_currentDate) != -1)
        m_currentUnit = unitForDate(m_currentDate);

    redraw();
}

void DateBar::DateBarWidget::setImageCollection(const DB::ImageInfoList &images)
{
    setImageDateCollection(QExplicitlySharedDataPointer<DB::ImageDateCollection>(
        new DB::ImageDateCollection(images)));
}

void DateBar::DateBarWidget::setImageDateCollection(const QExplicitlySharedDataPointer<DB::ImageDateCollection> &dates)
{
    m_dates = dates;
    if (m_doAutomaticRangeAdjustment && m_dates && !m_dates->lowerLimit().isNull()) {
        const Utilities::FastDateTime start = m_dates->lowerLimit();
        Utilities::FastDateTime end = m_dates->upperLimit();
        if (end.isNull())
            end = Utilities::FastDateTime::currentDateTime();

        m_currentDate = start;
        m_currentUnit = 0;
        // select suitable timeframe:
        setViewType(MinuteView, false);
        m_currentHandler->init(start);
        while (canZoomOut() && end > dateForUnit(numberOfUnits())) {
            m_tp = (ViewType)(m_tp - 1);
            setViewHandlerForType(m_tp);
            m_currentHandler->init(start);
        }
        // center range in datebar:
        int units = unitForDate(end);
        if (units != -1) {
            m_currentUnit = (numberOfUnits() - units) / 2;
        }
    }
    redraw();
}

void DateBar::DateBarWidget::drawHistograms(QPainter &p)
{
    QRect rect = barAreaGeometry();
    p.save();
    p.setClipping(true);
    p.setClipRect(rect);
    p.setPen(Qt::NoPen);

    // determine maximum image count within visible units
    int max = 0;
    for (int unit = 0; unit <= numberOfUnits(); unit++) {
        DB::ImageCount count = m_dates->count(rangeForUnit(unit));
        int cnt = count.mp_exact;
        if (m_includeFuzzyCounts)
            cnt += count.mp_rangeMatch;
        max = qMax(max, cnt);
    }

    // Calculate the font size for the largest number.
    QFont f = font();
    bool fontFound = false;
    for (int i = f.pointSize(); i >= 6; i -= 2) {
        f.setPointSize(i);
        QFontMetrics fontMetrics(f);
        int w = fontMetrics.horizontalAdvance(QString::number(max));
        if (w < rect.height() - 6) {
            p.setFont(f);
            fontFound = true;
            break;
        }
    }

    int unit = 0;
    const int minUnit = unitForDate(m_dates->lowerLimit()); // first non-empty unit
    const int maxUnit = (unitForDate(m_dates->upperLimit()) != -1) ? unitForDate(m_dates->upperLimit()) : numberOfUnits(); // last non-empty unit
    const bool linearScale = Settings::SettingsData::instance()->histogramUseLinearScale();
    for (int x = rect.x(); x + m_barWidth < rect.right(); x += m_barWidth, unit += 1) {
        if (unit < minUnit || unit > maxUnit) {
            Qt::BrushStyle style = Qt::SolidPattern;

            p.setBrush(QBrush(Qt::lightGray, style));
            p.drawRect(x, 1, m_barWidth, rect.height() + 2);
            continue;
        }
        const auto unitRange = rangeForUnit(unit);
        const DB::ImageCount count = m_dates->count(unitRange);
        int exactPx = 0;
        int rangePx = 0;
        if (max != 0) {
            double exactScaled;
            double rangeScaled;
            if (linearScale) {
                exactScaled = (double)count.mp_exact / max;
                rangeScaled = (double)count.mp_rangeMatch / max;
            } else {
                exactScaled = sqrt(count.mp_exact) / sqrt(max);
                rangeScaled = sqrt(count.mp_rangeMatch) / sqrt(max);
            }
            // convert to pixels:
            exactPx = (int)((double)(rect.height() - 2) * exactScaled);
            if (m_includeFuzzyCounts)
                rangePx = (int)((double)(rect.height() - 2) * rangeScaled);
        }

        Qt::BrushStyle style = Qt::SolidPattern;
        if (!isUnitSelected(unit) && hasSelection())
            style = Qt::Dense5Pattern;

        p.setBrush(QBrush(Qt::yellow, style));
        p.drawRect(x + 1, rect.bottom() - rangePx, m_barWidth - 2, rangePx);
        p.setBrush(QBrush(Qt::green, style));
        p.drawRect(x + 1, rect.bottom() - rangePx - exactPx, m_barWidth - 2, exactPx);

        // Draw the numbers, if they fit.
        if (fontFound) {
            int tot = count.mp_exact;
            if (m_includeFuzzyCounts)
                tot += count.mp_rangeMatch;
            p.save();
            p.translate(x + m_barWidth - 3, rect.bottom() - 2);
            p.rotate(-90);
            QFontMetrics fontMetrics(f);
            int w = fontMetrics.horizontalAdvance(QString::number(tot));
            if (w < exactPx + rangePx - 2) {
                // don't use a palette color here - otherwise it may have bad contrast with green and yellow:
                p.setPen(Qt::black);
                p.drawText(0, 0, QString::number(tot));
            }
            p.restore();
        }
    }

    p.restore();
}

void DateBar::DateBarWidget::scrollLeft()
{
    int scrollAmount = -SCROLL_AMOUNT;
    if (QGuiApplication::keyboardModifiers().testFlag(Qt::ShiftModifier))
        scrollAmount *= SCROLL_ACCELERATION;
    scroll(scrollAmount);
}

void DateBar::DateBarWidget::scrollRight()
{
    int scrollAmount = SCROLL_AMOUNT;
    if (QGuiApplication::keyboardModifiers().testFlag(Qt::ShiftModifier))
        scrollAmount *= SCROLL_ACCELERATION;
    scroll(scrollAmount);
}

void DateBar::DateBarWidget::scroll(int units)
{
    if ((m_dates->lowerLimit() <= dateForUnit(0) && units > 0)
        || (m_dates->upperLimit() > dateForUnit(numberOfUnits()) && units < 0)) {
        return;
    }

    m_currentDate = dateForUnit(units, m_currentDate);
    redraw();
    Q_EMIT dateSelected(currentDateRange(), includeFuzzyCounts());
}

void DateBar::DateBarWidget::drawFocusRectangle(QPainter &p)
{
    QRect rect = barAreaGeometry();
    p.save();
    int x = rect.left() + m_currentUnit * m_barWidth;
    QRect inner(QPoint(x - 1, BORDER_ABOVE_HISTOGRAM),
                QPoint(x + m_barWidth, BORDER_ABOVE_HISTOGRAM + m_barHeight - 1));

    p.setPen(QPen(palette().color(QPalette::Dark), 1));

    // Inner rect
    p.drawRect(inner);
    QRect outer = inner;
    outer.adjust(-2, -2, 2, 2);

    // Outer rect
    QRegion region = outer;
    region -= inner;
    p.setClipping(true);
    p.setClipRegion(region);

    QColor col = palette().highlight().color();
    if (!hasFocus())
        col = palette().window().color();

    p.setBrush(col);
    p.setPen(col);
    p.drawRect(outer);

    // Shadow below
    QRect shadow = outer;
    shadow.adjust(-1, -1, 1, 1);
    region = shadow;
    region -= outer;
    p.setPen(palette().color(QPalette::Shadow));
    p.setClipRegion(region);
    p.drawRect(shadow);

    // Light above
    QRect hide = shadow;
    hide.translate(1, 1);
    region = shadow;
    region -= hide;
    p.setPen(palette().color(QPalette::Light));
    p.setClipRegion(region);
    p.drawRect(shadow);

    p.restore();
}

void DateBar::DateBarWidget::zoomIn()
{
    if (!canZoomIn())
        return;
    zoom(+1);
}

void DateBar::DateBarWidget::zoomOut()
{
    if (!canZoomOut())
        return;
    zoom(-1);
}

void DateBar::DateBarWidget::zoom(int steps)
{
    ViewType tp = (ViewType)(m_tp + steps);
    const bool couldZoomIn = canZoomIn();
    const bool couldZoomOut = canZoomOut();
    setViewType(tp);
    if (couldZoomIn != canZoomIn())
        Q_EMIT zoomInEnabled(canZoomIn());
    if (couldZoomOut != canZoomOut())
        Q_EMIT zoomOutEnabled(canZoomOut());
}

void DateBar::DateBarWidget::mousePressEvent(QMouseEvent *event)
{
    if ((event->button() & (Qt::MiddleButton | Qt::LeftButton)) == 0 || event->position().x() > barAreaGeometry().right() || event->position().x() < barAreaGeometry().left())
        return;

    if ((event->button() & Qt::MiddleButton)
        || event->modifiers() & Qt::ControlModifier) {
        m_currentMouseHandler = m_barDragHandler;
    } else {
        bool onBar = event->position().y() > barAreaGeometry().bottom();
        if (onBar)
            m_currentMouseHandler = m_selectionHandler;
        else {
            m_currentMouseHandler = m_focusItemDragHandler;
        }
    }
    m_currentMouseHandler->mousePressEvent(event->position().x());
    Q_EMIT dateSelected(currentDateRange(), includeFuzzyCounts());
    showStatusBarTip(event->pos());
    redraw();
}

void DateBar::DateBarWidget::mouseReleaseEvent(QMouseEvent *)
{
    if (m_currentMouseHandler == nullptr)
        return;

    m_currentMouseHandler->endAutoScroll();
    m_currentMouseHandler->mouseReleaseEvent();
    m_currentMouseHandler = nullptr;
}

void DateBar::DateBarWidget::mouseMoveEvent(QMouseEvent *event)
{
    showStatusBarTip(event->pos());

    if (m_currentMouseHandler == nullptr)
        return;

    if ((event->buttons() & (Qt::MiddleButton | Qt::LeftButton)) == 0)
        return;

    m_currentMouseHandler->endAutoScroll();
    m_currentMouseHandler->mouseMoveEvent(event->pos().x());
}

QRect DateBar::DateBarWidget::barAreaGeometry() const
{
    QRect barArea;
    barArea.setTopLeft(QPoint(BORDER_AROUND_WIDGET, BORDER_ABOVE_HISTOGRAM));
    barArea.setRight(width() - BORDER_AROUND_WIDGET - 2 * BUTTON_WIDTH - 2 * 3); // 2 pixels between button and bar + 1 pixel as the pen is one pixel
    barArea.setHeight(m_barHeight);
    return barArea;
}

int DateBar::DateBarWidget::numberOfUnits() const
{
    return barAreaGeometry().width() / m_barWidth - 1;
}

void DateBar::DateBarWidget::setHistogramBarSize(const QSize &size)
{
    m_barWidth = size.width();
    m_barHeight = size.height();
    m_currentUnit = numberOfUnits() / 2;
    Q_ASSERT(parentWidget());
    updateGeometry();
    Q_ASSERT(parentWidget());
    placeAndSizeButtons();
    redraw();
}

void DateBar::DateBarWidget::setIncludeFuzzyCounts(bool b)
{
    m_includeFuzzyCounts = b;
    redraw();
    if (hasSelection())
        emitRangeSelection(m_selectionHandler->dateRange());

    Q_EMIT dateSelected(currentDateRange(), includeFuzzyCounts());
}

DB::ImageDate DateBar::DateBarWidget::rangeAt(const QPoint &p)
{
    int unit = (p.x() - barAreaGeometry().x()) / m_barWidth;
    return rangeForUnit(unit);
}

DB::ImageDate DateBar::DateBarWidget::rangeForUnit(int unit)
{
    Utilities::FastDateTime toUnit = dateForUnit(unit + 1).addSecs(-1);
    return DB::ImageDate(dateForUnit(unit), toUnit);
}

bool DateBar::DateBarWidget::includeFuzzyCounts() const
{
    return m_includeFuzzyCounts;
}

KActionCollection *DateBar::DateBarWidget::actions()
{
    return m_actionCollection;
}

bool DateBar::DateBarWidget::canZoomIn() const
{
    return (m_tp != MinuteView);
}

bool DateBar::DateBarWidget::canZoomOut() const
{
    return (m_tp != DecadeView);
}

void DateBar::DateBarWidget::centerDateRange(const DB::ImageDate &range)
{
    centerDateRange(range.start(), range.end());
}

void DateBar::DateBarWidget::centerDateRange(const Utilities::FastDateTime &min, const Utilities::FastDateTime &max)
{
    m_currentDate = min;
    // update reference frame for unitForDate:
    m_currentHandler->init(m_currentDate);
    const int maxUnit = unitForDate(max);
    if (maxUnit != -1) {
        // center selection if it fits within the date bar
        const int paddingUnits = (numberOfUnits() - maxUnit) / 2;
        m_currentUnit = paddingUnits;
    }
}

void DateBar::DateBarWidget::contextMenuEvent(QContextMenuEvent *event)
{
    if (!m_contextMenu) {
        m_contextMenu = new QMenu(this);
        QAction *action = new QAction(i18n("Show Ranges"), this);
        action->setCheckable(true);
        m_contextMenu->addAction(action);
        action->setChecked(m_includeFuzzyCounts);
        connect(action, &QAction::toggled, this, &DateBarWidget::setIncludeFuzzyCounts);

        action = new QAction(i18n("Show Resolution Indicator"), this);
        action->setCheckable(true);
        m_contextMenu->addAction(action);
        action->setChecked(m_showResolutionIndicator);
        connect(action, &QAction::toggled, this, &DateBarWidget::setShowResolutionIndicator);
    }

    m_contextMenu->exec(event->globalPos());
    event->setAccepted(true);
}

QRect DateBar::DateBarWidget::tickMarkGeometry() const
{
    QRect rect;
    rect.setTopLeft(barAreaGeometry().bottomLeft());
    rect.setWidth(barAreaGeometry().width());
    rect.setHeight(12);
    return rect;
}

void DateBar::DateBarWidget::drawResolutionIndicator(QPainter &p, int *leftEdge)
{
    QRect rect = dateAreaGeometry();

    // For real small bars, we do not want to show the resolution.
    if (rect.width() < 400 || !m_showResolutionIndicator) {
        *leftEdge = rect.right();
        return;
    }

    QString text = m_currentHandler->unitText();
    QFontMetrics fontMetrics(font());
    int textWidth = fontMetrics.horizontalAdvance(text);
    int height = fontMetrics.height();

    int endUnitPos = rect.right() - textWidth - ARROW_LENGTH - 3;
    // Round to nearest unit mark
    endUnitPos = ((endUnitPos - rect.left()) / m_barWidth) * m_barWidth + rect.left();
    int startUnitPos = endUnitPos - m_barWidth;
    int midLine = rect.top() + height / 2;

    p.save();
    p.setPen(palette().windowText().color());

    // draw arrows
    drawArrow(p, QPoint(startUnitPos - ARROW_LENGTH, midLine), QPoint(startUnitPos, midLine));
    drawArrow(p, QPoint(endUnitPos + ARROW_LENGTH, midLine), QPoint(endUnitPos, midLine));
    p.drawLine(startUnitPos, rect.top(), startUnitPos, rect.top() + height);
    p.drawLine(endUnitPos, rect.top(), endUnitPos, rect.top() + height);

    // draw text
    QFontMetrics fm(font());
    p.drawText(endUnitPos + ARROW_LENGTH + 3, rect.top(), fm.horizontalAdvance(text), fm.height(), Qt::TextSingleLine, text);
    p.restore();

    *leftEdge = startUnitPos - ARROW_LENGTH - 3;
}

QRect DateBar::DateBarWidget::dateAreaGeometry() const
{
    QRect rect = tickMarkGeometry();
    rect.setTop(rect.bottom() + 2);
    rect.setHeight(QFontMetrics(font()).height());
    return rect;
}

void DateBar::DateBarWidget::drawArrow(QPainter &p, const QPoint &start, const QPoint &end)
{
    p.save();
    p.drawLine(start, end);

    QPoint diff = QPoint(end.x() - start.x(), end.y() - start.y());
    double dx = diff.x();
    double dy = diff.y();

    if (dx != 0 || dy != 0) {
        if (dy < 0)
            dx = -dx;
        double angle = acos(dx / sqrt(dx * dx + dy * dy)) * 180. / M_PI;
        if (dy < 0)
            angle += 180.;

        // angle is now the angle of the line.

        angle = angle + 180 - 15;
        p.translate(end.x(), end.y());
        p.rotate(angle);
        p.drawLine(QPoint(0, 0), QPoint(10, 0));

        p.rotate(30);
        p.drawLine(QPoint(0, 0), QPoint(10, 0));
    }

    p.restore();
}

void DateBar::DateBarWidget::setShowResolutionIndicator(bool b)
{
    m_showResolutionIndicator = b;
    redraw();
}

void DateBar::DateBarWidget::setAutomaticRangeAdjustment(bool b)
{
    m_doAutomaticRangeAdjustment = b;
}

void DateBar::DateBarWidget::updateArrowState()
{
    m_leftArrow->setEnabled(m_dates->lowerLimit() <= dateForUnit(0));
    m_rightArrow->setEnabled(m_dates->upperLimit() > dateForUnit(numberOfUnits()));
}

DB::ImageDate DateBar::DateBarWidget::currentDateRange() const
{
    return DB::ImageDate(dateForUnit(m_currentUnit), dateForUnit(m_currentUnit + 1));
}

void DateBar::DateBarWidget::showStatusBarTip(const QPoint &pos)
{
    DB::ImageDate range = rangeAt(pos);
    DB::ImageCount count = m_dates->count(range);

    QString cnt;
    if (count.mp_rangeMatch != 0 && includeFuzzyCounts())
        cnt = i18ncp("@info:status images that fall in the given date range", "1 exact", "%1 exact", count.mp_exact)
            + i18ncp("@info:status additional images captured in a date range that overlaps with the given date range,", " + 1 range", " + %1 ranges", count.mp_rangeMatch)
            + i18ncp("@info:status total image count", " = 1 total", " = %1 total", count.mp_exact + count.mp_rangeMatch);
    else
        cnt = i18ncp("@info:status image count", "%1 image/video", "%1 images/videos", count.mp_exact);

    QString res = i18nc("@info:status Time range vs. image count (e.g. 'Jun 2012 | 4 images/videos').", "%1 | %2", range.toString(), cnt);

    static QString lastTip;
    if (lastTip != res)
        Q_EMIT toolTipInfo(res);
    lastTip = res;
}

void DateBar::DateBarWidget::placeAndSizeButtons()
{
    m_zoomIn->setFixedSize(BUTTON_WIDTH, BUTTON_WIDTH);
    m_zoomOut->setFixedSize(BUTTON_WIDTH, BUTTON_WIDTH);
    m_rightArrow->setFixedSize(QSize(BUTTON_WIDTH, m_barHeight));
    m_leftArrow->setFixedSize(QSize(BUTTON_WIDTH, m_barHeight));

    m_rightArrow->move(size().width() - m_rightArrow->width() - BORDER_AROUND_WIDGET, BORDER_ABOVE_HISTOGRAM);
    m_leftArrow->move(m_rightArrow->pos().x() - m_leftArrow->width() - 2, BORDER_ABOVE_HISTOGRAM);

    int x = m_leftArrow->pos().x();
    int y = height() - BUTTON_WIDTH;
    m_zoomOut->move(x, y);

    x = m_rightArrow->pos().x();
    m_zoomIn->move(x, y);

    m_cancelSelection->setFixedSize(BUTTON_WIDTH, BUTTON_WIDTH);
    m_cancelSelection->move(0, y);
}

void DateBar::DateBarWidget::keyPressEvent(QKeyEvent *event)
{
    int offset = 0;

    switch (event->key()) {
    case Qt::Key_Left:
        offset = -1;
        break;
    case Qt::Key_Right:
        offset = 1;
        break;
    case Qt::Key_PageDown:
        offset = -10;
        break;
    case Qt::Key_PageUp:
        offset = 10;
        break;
    case Qt::Key_Plus:
        if (canZoomIn())
            zoom(1);
        return;
    case Qt::Key_Minus:
        if (canZoomOut())
            zoom(-1);
        return;
    case Qt::Key_Escape:
        clearSelection();
        return;
    default:
        return;
    }

    const bool selectionMode = event->modifiers() & Qt::ShiftModifier;

    Utilities::FastDateTime newDate = dateForUnit(offset, m_currentDate);
    if ((offset < 0 && newDate >= m_dates->lowerLimit()) || (offset > 0 && newDate <= m_dates->upperLimit())) {
        m_currentDate = newDate;
        m_currentUnit += offset;
        if (m_currentUnit < 0)
            m_currentUnit = 0;
        if (m_currentUnit > numberOfUnits())
            m_currentUnit = numberOfUnits();

        if (selectionMode) {
            m_selectionHandler->setOrExtendSelection(newDate);
        } else if (!currentSelection().includes(m_currentDate))
            clearSelection();
    }
    redraw();
    Q_EMIT dateSelected(currentDateRange(), includeFuzzyCounts());
}

void DateBar::DateBarWidget::focusInEvent(QFocusEvent *)
{
    redraw();
}

void DateBar::DateBarWidget::focusOutEvent(QFocusEvent *)
{
    redraw();
}

int DateBar::DateBarWidget::unitAtPos(int x) const
{
    const bool invalidOffset_before = x - barAreaGeometry().left() < 0;
    const bool invalidOffset_after = x - barAreaGeometry().left() > barAreaGeometry().width();
    if (invalidOffset_before || invalidOffset_after) {
        return -1;
    }

    return (x - barAreaGeometry().left()) / m_barWidth;
}

Utilities::FastDateTime DateBar::DateBarWidget::dateForUnit(int unit, const Utilities::FastDateTime &offset) const
{
    return m_currentHandler->date(unit, offset);
}

bool DateBar::DateBarWidget::isUnitSelected(int unit) const
{
    Utilities::FastDateTime minDate = m_selectionHandler->min();
    Utilities::FastDateTime maxDate = m_selectionHandler->max();
    Utilities::FastDateTime date = dateForUnit(unit);
    return (minDate <= date && date < maxDate && !minDate.isNull());
}

bool DateBar::DateBarWidget::hasSelection() const
{
    return !m_selectionHandler->min().isNull();
}

DB::ImageDate DateBar::DateBarWidget::currentSelection() const
{
    return DB::ImageDate(m_selectionHandler->min(), m_selectionHandler->max());
}

void DateBar::DateBarWidget::clearSelection()
{
    if (m_selectionHandler->hasSelection()) {
        m_selectionHandler->clearSelection();
        Q_EMIT dateRangeCleared();
        Q_EMIT dateRangeSelected(false);
        redraw();
    }
}

void DateBar::DateBarWidget::emitRangeSelection(const DB::ImageDate &range)
{
    Q_EMIT dateRangeChange(range);
    Q_EMIT dateRangeSelected(true);
}

int DateBar::DateBarWidget::unitForDate(const Utilities::FastDateTime &date) const
{
    for (int unit = 0; unit < numberOfUnits(); ++unit) {
        if (m_currentHandler->date(unit) <= date && date < m_currentHandler->date(unit + 1))
            return unit;
    }
    return -1;
}

void DateBar::DateBarWidget::emitDateSelected()
{
    Q_EMIT dateSelected(currentDateRange(), includeFuzzyCounts());
}

void DateBar::DateBarWidget::wheelEvent(QWheelEvent *e)
{
    const auto angleDelta = e->angleDelta();
    const bool isHorizontal = (qAbs(angleDelta.x()) > qAbs(angleDelta.y()));
    const int delta = isHorizontal ? angleDelta.x() : angleDelta.y();
    if (e->modifiers() & Qt::ControlModifier) {
        if (delta > 0)
            zoomIn();
        else
            zoomOut();
        return;
    }
    int scrollAmount = delta > 0 ? SCROLL_AMOUNT : -SCROLL_AMOUNT;
    if (e->modifiers() & Qt::ShiftModifier)
        scrollAmount *= SCROLL_ACCELERATION;
    scroll(scrollAmount);
}

#include "moc_DateBarWidget.cpp"

// vi:expandtab:tabstop=4 shiftwidth=4:
