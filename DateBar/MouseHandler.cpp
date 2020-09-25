/* Copyright (C) 2003-2019 The KPhotoAlbum Development Team

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
    int unit = m_dateBar->unitAtPos(x);
    m_start = m_dateBar->dateForUnit(unit);
    m_end = m_dateBar->dateForUnit(unit + 1);
}

void DateBar::SelectionHandler::mouseMoveEvent(int x)
{
    int unit = m_dateBar->unitAtPos(x);
    Utilities::FastDateTime date = m_dateBar->dateForUnit(unit);
    if (m_start < date)
        m_end = m_dateBar->dateForUnit(unit + 1);
    else
        m_end = date;
    m_dateBar->redraw();
}

DateBar::FocusItemDragHandler::FocusItemDragHandler(DateBarWidget *dateBar)
    : MouseHandler(dateBar)
{
}

void DateBar::FocusItemDragHandler::mousePressEvent(int x)
{
    m_dateBar->m_currentUnit = m_dateBar->unitAtPos(x);
    m_dateBar->m_currentDate = m_dateBar->dateForUnit(m_dateBar->m_currentUnit);
    if (m_dateBar->hasSelection() && !m_dateBar->currentSelection().includes(m_dateBar->m_currentDate))
        m_dateBar->clearSelection();
}

void DateBar::FocusItemDragHandler::mouseMoveEvent(int x)
{
    int oldUnit = m_dateBar->m_currentUnit;
    int newUnit = (x - m_dateBar->barAreaGeometry().left()) / m_dateBar->m_barWidth;

    // Don't scroll further down than the last image
    // We use oldUnit here, to ensure that we scroll all the way to the end
    // better scroll a bit over than not all the way.
    if ((newUnit > oldUnit && m_dateBar->dateForUnit(oldUnit) > m_dateBar->m_dates->upperLimit()) || (newUnit < oldUnit && m_dateBar->dateForUnit(oldUnit) < m_dateBar->m_dates->lowerLimit()))
        return;
    m_dateBar->m_currentUnit = newUnit;

    static double rest = 0;
    if (m_dateBar->m_currentUnit < 0 || m_dateBar->m_currentUnit > m_dateBar->numberOfUnits()) {
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
{
}

void DateBar::BarDragHandler::mousePressEvent(int x)
{
    m_movementOffset = m_dateBar->m_currentUnit * m_dateBar->m_barWidth - (x - m_dateBar->barAreaGeometry().left());
}

void DateBar::BarDragHandler::mouseMoveEvent(int x)
{
    int oldUnit = m_dateBar->m_currentUnit;
    int newUnit = (x + m_movementOffset - m_dateBar->barAreaGeometry().left()) / m_dateBar->m_barWidth;

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
    m_dateBar->redraw();
    m_dateBar->emitDateSelected();
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

// vi:expandtab:tabstop=4 shiftwidth=4:
