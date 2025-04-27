/*
 * SPDX-FileCopyrightText: 2025 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
 */

// SPDX-FileCopyrightText: 2003-2022 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DATEBARMOUSEHANDLER_H
#define DATEBARMOUSEHANDLER_H

#include <DB/ImageDate.h>

#include <Utilities/FastDateTime.h>
#include <QObject>

namespace DB
{
class ImageDate;
}

class QTimer;
namespace DateBar
{
class DateBarWidget;

class MouseHandler : public QObject
{
    Q_OBJECT
public:
    explicit MouseHandler(DateBarWidget *dateBar);
    virtual void mousePressEvent(int x) = 0;
    virtual void mouseMoveEvent(int x) = 0;
    virtual void mouseReleaseEvent() { };
    void startAutoScroll();
    void endAutoScroll();

protected Q_SLOTS:
    void autoScroll();

protected:
    DateBarWidget *m_dateBar;

private:
    QTimer *m_autoScrollTimer;
};

/**
 * @brief The FocusItemDragHandler class handles mouse events for the histogram part of the DateBar.
 * I.e. the user clicks on the histogram part of the DateBar to jump to the corresponding date/time.
 */
class FocusItemDragHandler : public MouseHandler
{
public:
    explicit FocusItemDragHandler(DateBarWidget *dateBar);
    void mousePressEvent(int x) override;
    void mouseMoveEvent(int x) override;
};

/**
 * @brief The BarDragHandler class handles moving the DateBar histogram by dragging it.
 * If a time range selection was set (by the SelectionHandler) and the user clicks outside that range, the selection is cleared.
 */
class BarDragHandler : public MouseHandler
{
public:
    explicit BarDragHandler(DateBarWidget *);
    void mousePressEvent(int x) override;
    void mouseMoveEvent(int x) override;
    void mouseReleaseEvent() override;

private:
    int m_movementOffset;
};

/**
 * @brief The SelectionHandler class handles mouse events for the area below the DateBar histogram.
 * I.e. the user clicks somewhere in the area below the histogram part of the DateBar and selects a time range by dragging the mouse.
 * When the user releases the mouse button, the selection is updated to the selected area.
 */
class SelectionHandler : public MouseHandler
{
public:
    explicit SelectionHandler(DateBarWidget *);
    void mousePressEvent(int x) override;
    void mouseMoveEvent(int x) override;
    void mouseReleaseEvent() override;
    Utilities::FastDateTime min() const;
    Utilities::FastDateTime max() const;
    DB::ImageDate dateRange() const;
    void clearSelection();
    bool hasSelection() const;
    /**
     * @brief setOrExtendSelection extends the selection to the given date.
     * If the SelectionHandler does not currently have a selection, it selects the unit corrensponding to the date.
     * @param date a valid FastDateTime
     */
    void setOrExtendSelection(const Utilities::FastDateTime &date);

private:
    Utilities::FastDateTime m_start;
    Utilities::FastDateTime m_end;
};
}

#endif /* DATEBARMOUSEHANDLER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
