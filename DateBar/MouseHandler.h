// SPDX-FileCopyrightText: 2003-2022 Jesper K. Pedersen <blackie@kde.org>
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
    virtual void mouseReleaseEvent() {};
    void startAutoScroll();
    void endAutoScroll();

protected Q_SLOTS:
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
    Utilities::FastDateTime min() const;
    Utilities::FastDateTime max() const;
    DB::ImageDate dateRange() const;
    void clearSelection();
    bool hasSelection() const;

private:
    Utilities::FastDateTime m_start;
    Utilities::FastDateTime m_end;
};
}

#endif /* DATEBARMOUSEHANDLER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
