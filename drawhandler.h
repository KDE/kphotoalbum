/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef DRAWHANDLER_H
#define DRAWHANDLER_H

#include "displayareahandler.h"
#include <qobject.h>
#include "drawlist.h"

class QMouseEvent;
class QPixmap;
class QPoint;
class Draw;
class QPainter;

class DrawHandler :public DisplayAreaHandler {
    Q_OBJECT

public:
    DrawHandler( DisplayArea* display );
    virtual bool mousePressEvent ( QMouseEvent* e, const QPoint& unTranslatedPos, double scaleFactor );
    virtual bool mouseReleaseEvent ( QMouseEvent* e, const QPoint& unTranslatedPos, double scaleFactor );
    virtual bool mouseMoveEvent ( QMouseEvent* e, const QPoint& unTranslatedPos, double scaleFactor );
    DrawList drawList() const;
    void setDrawList( const DrawList& );
    bool hasDrawings() const;

    void drawAll( QPainter& );
    void stopDrawing();

public slots:
    void slotLine();
    void slotRectangle();
    void slotCircle();
    void slotSelect();
    void cut();

signals:
    void redraw();
    void active();

protected:
    Draw* createTool();
    Draw* findShape( const QPoint& );
    void setupPainter( QPainter* painter );

private:
    enum Tool { Select, Line, Rectangle, Circle, None};
    Tool _tool;
    Draw* _activeTool;
    DrawList _drawings;


};

#endif /* DRAWHANDLER_H */

