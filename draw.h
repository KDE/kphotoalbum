/*
 *  Copyright (c) 2003 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

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

class Draw
{
public:
    Draw( QWidget* widget = 0 ) :_widget( widget ) {}
    void startDraw( QMouseEvent* );
    virtual void draw( QPainter&, QMouseEvent* );
    virtual PointList anchorPoints() = 0;
    QPoint w2g( const QPoint& ); // widget2generic
    QPoint g2w( const QPoint& );
    virtual Draw* clone() = 0;
    virtual QDomElement save( QDomDocument doc ) = 0;
    void setWidget( QWidget* widget );

protected:
    QPoint _startPos;
    QPoint _lastPos;
    QWidget* _widget;
    void saveDrawAttr( QDomElement* elm );
    void readDrawAttr( QDomElement elm );
};

#endif /* DRAW_H */

