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

#ifndef DRAW_H
#define DRAW_H
class QMouseEvent;
class QPainter;
class QWidget;
#include <qpoint.h>
#include <qvaluelist.h>
#include <qdom.h>

typedef QValueList<QPoint> PointList;
typedef QValueList<QPoint>::Iterator PointListIterator;

namespace Viewer
{

class Draw
{
public:
    void startDraw( QMouseEvent* );
    virtual void draw( QPainter*, QMouseEvent* );
    virtual PointList anchorPoints() = 0;
    virtual Draw* clone() = 0;
    virtual QDomElement save( QDomDocument doc ) = 0;
    void setCoordinates(const QPoint& p0, const QPoint& p1);
    QPoint getStartPoint() const { return _startPos; }
    QPoint getEndPoint() const { return _lastPos; }

protected:
    QPoint _startPos;
    QPoint _lastPos;
    void saveDrawAttr( QDomElement* elm );
    void readDrawAttr( QDomElement elm );
};

}

#endif /* DRAW_H */

