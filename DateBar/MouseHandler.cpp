// SPDX-FileCopyrightText: 2003 - 2019 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2022 - 2025 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "MouseHandler.h"

#include "DateBarWidget.h"

#include <DB/ImageDateCollection.h>

#include <math.h>
#include <qcursor.h>
#include <qtimer.h>

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
DateBar::MouseHandler::MouseHandler(DateBarWidget *dateBar)
    : QObject(dateBar)
    , m_dateBar(dateBar)
{
    m_autoScrollTimer = new QTimer(this);
    connect(m_autoScrollTimer, &QTimer::timeout, this, &SelectionHandler::autoScroll);
}

void DateBar::MouseHandler::autoScroll()
{
    mouseMoveEvent(m_dateBar->mapFromGlobal(QCursor::pos()).x());
}

void DateBar::MouseHandler::startAutoScroll()
{
    m_autoScrollTimer->start(100);
}

void DateBar::MouseHandler::endAutoScroll()
{
    m_autoScrollTimer->stop();
}

DateBar::SelectionHandler::SelectionHandler(DateBarWidget *dateBar)
    : MouseHandler(dateBar)
{
}

void DateBar::SelectionHandler::mousePressEvent(int x)
{
    const int unit = m_dateBar->unitAtPos(x);
    m_currentUnit = unit;
    if (unit < 0)
        return;

    m_start = m_dateBar->dateForUnit(unit);
    m_end = m_dateBar->dateForUnit(unit + 1).addSecs(-1);
}

void DateBar::SelectionHandler::mouseMoveEvent(int x)
{
    const int unit = m_dateBar->unitAtPos(x);
    if (unit < 0 || unit == m_currentUnit)
        return;
    m_currentUnit = unit;

    Utilities::FastDateTime date = m_dateBar->dateForUnit(unit);
    if (m_start < date) {
        m_end = m_dateBar->dateForUnit(unit + 1).addSecs(-1);
    } else {
        m_end = date;
    }
    m_dateBar->redraw();
}

DateBar::FocusItemDragHandler::FocusItemDragHandler(DateBarWidget *dateBar)
    : MouseHandler(dateBar)
{
}

void DateBar::FocusItemDragHandler::mousePressEvent(int x)
{
    const int unit = m_dateBar->unitAtPos(x);
    if (unit < 0)
        return;

    m_dateBar->m_currentUnit = unit;
    m_dateBar->m_currentDate = m_dateBar->dateForUnit(unit);
    if (m_dateBar->hasSelection() && !m_dateBar->currentSelection().includes(m_dateBar->m_currentDate))
        m_dateBar->clearSelection();
}

void DateBar::FocusItemDragHandler::mouseMoveEvent(int x)
{
    const int oldUnit = m_dateBar->m_currentUnit;
    const int newUnit = (x - m_dateBar->barAreaGeometry().left()) / m_dateBar->m_barWidth;
    if (oldUnit == newUnit)
        return;

    // Don't scroll further down than the last image
    // We use oldUnit here, to ensure that we scroll all the way to the end
    // better scroll a bit over than not all the way.
    if ((newUnit > oldUnit && m_dateBar->dateForUnit(oldUnit) > m_dateBar->m_dates->upperLimit()) || (newUnit < oldUnit && m_dateBar->dateForUnit(oldUnit) < m_dateBar->m_dates->lowerLimit()))
        return;

    // don't scroll past the selection
    if (m_dateBar->hasSelection() && !m_dateBar->currentSelection().includes(m_dateBar->dateForUnit(newUnit)))
        return;

    m_dateBar->m_currentUnit = newUnit;

    if (m_dateBar->m_currentUnit < 0 || m_dateBar->m_currentUnit > m_dateBar->numberOfUnits()) {
        static double rest = 0;
        // Slow down scrolling outside date bar.
        double newUnit = oldUnit + (m_dateBar->m_currentUnit - oldUnit) / 4.0 + rest;
        m_dateBar->m_currentUnit = (int)floor(newUnit);
        rest = newUnit - m_dateBar->m_currentUnit;
        startAutoScroll();
    }

    m_dateBar->m_currentDate = m_dateBar->dateForUnit(m_dateBar->m_currentUnit);
    m_dateBar->m_currentUnit = qMax(m_dateBar->m_currentUnit, 0);
    m_dateBar->m_currentUnit = qMin(m_dateBar->m_currentUnit, m_dateBar->numberOfUnits());
    m_dateBar->redraw();
    m_dateBar->emitDateSelected();
}

DateBar::BarDragHandler::BarDragHandler(DateBarWidget *dateBar)
    : MouseHandler(dateBar)
    , m_movementOffset(0)
{
}

void DateBar::BarDragHandler::mousePressEvent(int x)
{
    m_movementOffset = m_dateBar->m_currentUnit * m_dateBar->m_barWidth - (x - m_dateBar->barAreaGeometry().left());
}

void DateBar::BarDragHandler::mouseMoveEvent(int x)
{
    const int oldUnit = m_dateBar->m_currentUnit;
    const int newUnit = (x + m_movementOffset - m_dateBar->barAreaGeometry().left()) / m_dateBar->m_barWidth;

    if (oldUnit == newUnit)
        return;

    // Don't scroll further down than the last image
    // We use oldUnit here, to ensure that we scroll all the way to the end
    // better scroll a bit over than not all the way.
    if ((newUnit > oldUnit && m_dateBar->dateForUnit(0) < m_dateBar->m_dates->lowerLimit()) || (newUnit < oldUnit && m_dateBar->dateForUnit(m_dateBar->numberOfUnits()) > m_dateBar->m_dates->upperLimit()))
        return;

    m_dateBar->m_currentUnit = newUnit;

    if (m_dateBar->m_currentUnit < 0) {
        m_dateBar->m_currentDate = m_dateBar->dateForUnit(-m_dateBar->m_currentUnit);
        m_dateBar->m_currentUnit = 0;
        m_movementOffset = m_dateBar->barAreaGeometry().left() - x;
    } else if (m_dateBar->m_currentUnit > m_dateBar->numberOfUnits()) {
        int diff = m_dateBar->numberOfUnits() - m_dateBar->m_currentUnit;
        m_dateBar->m_currentDate = m_dateBar->dateForUnit(m_dateBar->numberOfUnits() + diff);
        m_dateBar->m_currentUnit = m_dateBar->numberOfUnits();
        m_movementOffset = (m_dateBar->numberOfUnits() * m_dateBar->m_barWidth) - x + m_dateBar->m_barWidth / 2;
    }
    m_dateBar->redraw(DateBarWidget::RedrawMode::Fast);
    m_dateBar->emitDateSelected();
}

void DateBar::BarDragHandler::mouseReleaseEvent()
{
    // after dragging ends, we need a full redraw
    m_dateBar->redraw();
}

Utilities::FastDateTime DateBar::SelectionHandler::min() const
{
    if (m_start < m_end)
        return m_start;
    else
        return m_end;
}

Utilities::FastDateTime DateBar::SelectionHandler::max() const
{
    if (m_start >= m_end)
        return m_dateBar->dateForUnit(1, m_start);
    else
        return m_end;
}

void DateBar::SelectionHandler::clearSelection()
{
    m_start = Utilities::FastDateTime();
    m_end = Utilities::FastDateTime();
}

void DateBar::SelectionHandler::mouseReleaseEvent()
{
    m_currentUnit = -1;
    m_dateBar->emitRangeSelection(dateRange());
}

DB::ImageDate DateBar::SelectionHandler::dateRange() const
{
    return DB::ImageDate(min(), max());
}

bool DateBar::SelectionHandler::hasSelection() const
{
    return min().isValid();
}

void DateBar::SelectionHandler::setOrExtendSelection(const Utilities::FastDateTime &date)
{
    if (hasSelection()) {
        if (date < m_start) {
            m_start = date;
            m_dateBar->emitRangeSelection(dateRange());
        } else if (date > m_end) {
            const auto unit = m_dateBar->unitForDate(date);
            m_end = m_dateBar->dateForUnit(unit + 1).addSecs(-1);
            m_dateBar->emitRangeSelection(dateRange());
        }
    } else {
        m_start = date;
        const auto unit = m_dateBar->unitForDate(date);
        m_end = m_dateBar->dateForUnit(unit + 1).addSecs(-1);
        m_dateBar->emitRangeSelection(dateRange());
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_MouseHandler.cpp"
