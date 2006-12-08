/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
#ifndef DATEBARMOUSEHANDLER_H
#define DATEBARMOUSEHANDLER_H
#include <qevent.h>
#include <qobject.h>
#include <qdatetime.h>
#include <kdemacros.h>
#include <kdeversion.h>
#include "DB/ImageDate.h"

#if ! KDE_IS_VERSION(3,3,90)
#undef KDE_EXPORT
#define KDE_EXPORT
#endif

namespace DateBar {
class DateBarWidget;

    class KDE_EXPORT MouseHandler : public QObject
    {
        Q_OBJECT
    public:
        MouseHandler( DateBarWidget* dateBar );
        virtual void mousePressEvent( int x ) = 0;
        virtual void mouseMoveEvent( int x ) = 0;
        virtual void mouseReleaseEvent() {};
        void startAutoScroll();
        void endAutoScroll();

    protected slots:
        void autoScroll();

    protected:
        DateBarWidget* _dateBar;

    private:
        QTimer* _autoScrollTimer;
    };



    class KDE_EXPORT FocusItemDragHandler : public MouseHandler
    {
    public:
        FocusItemDragHandler( DateBarWidget* dateBar );
        void mousePressEvent( int x );
        void mouseMoveEvent( int x );
    };



    class KDE_EXPORT BarDragHandler : public MouseHandler
    {
    public:
        BarDragHandler( DateBarWidget* );
        void mousePressEvent( int x );
        void mouseMoveEvent(  int x );
    private:
        int _movementOffset;
    };



    class KDE_EXPORT SelectionHandler : public MouseHandler
    {
    public:
        SelectionHandler( DateBarWidget* );
        void mousePressEvent( int x );
        void mouseMoveEvent( int x );
        virtual void mouseReleaseEvent();
        QDateTime min() const;
        QDateTime max() const;
        DB::ImageDate dateRange() const;
        void clearSelection();
        bool hasSelection() const;
    private:
        QDateTime _start;
        QDateTime _end;
    };
}

#endif /* DATEBARMOUSEHANDLER_H */

