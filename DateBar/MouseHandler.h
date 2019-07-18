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
#ifndef DATEBARMOUSEHANDLER_H
#define DATEBARMOUSEHANDLER_H
#include "DB/ImageDate.h"
#include <QObject>
#include <qdatetime.h>

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
    virtual void mouseReleaseEvent() {};
    void startAutoScroll();
    void endAutoScroll();

protected slots:
    void autoScroll();

protected:
    DateBarWidget *m_dateBar;

private:
    QTimer *m_autoScrollTimer;
};

class FocusItemDragHandler : public MouseHandler
{
public:
    explicit FocusItemDragHandler(DateBarWidget *dateBar);
    void mousePressEvent(int x) override;
    void mouseMoveEvent(int x) override;
};

class BarDragHandler : public MouseHandler
{
public:
    explicit BarDragHandler(DateBarWidget *);
    void mousePressEvent(int x) override;
    void mouseMoveEvent(int x) override;

private:
    int m_movementOffset;
};

class SelectionHandler : public MouseHandler
{
public:
    explicit SelectionHandler(DateBarWidget *);
    void mousePressEvent(int x) override;
    void mouseMoveEvent(int x) override;
    void mouseReleaseEvent() override;
    QDateTime min() const;
    QDateTime max() const;
    DB::ImageDate dateRange() const;
    void clearSelection();
    bool hasSelection() const;

private:
    QDateTime m_start;
    QDateTime m_end;
};
}

#endif /* DATEBARMOUSEHANDLER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
