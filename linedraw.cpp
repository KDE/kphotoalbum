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

#include "linedraw.h"
#include <qpainter.h>
#include <qevent.h>
#include <math.h>
LineDraw::LineDraw( QWidget* widget ) :Draw( widget )
{

}

void LineDraw::draw( QPainter& painter, QMouseEvent* event )
{
    Draw::draw( painter, event );

    painter.save();
    painter.drawLine( g2w(_startPos), g2w(_lastPos) );

    QPoint diff = g2w( QPoint( _lastPos.x() - _startPos.x(), _lastPos.y() - _startPos.y() ) );
    double dx = diff.x();
    double dy = diff.y();

    if ( dx != 0 || dy != 0 ) {
        if( dy < 0 ) dx = -dx;
        double angle = acos(dx/sqrt( dx*dx+dy*dy ))*180./M_PI;
        if( dy < 0 ) angle += 180.;

        // angle is now the angle of the line.

        angle = angle + 180 - 15;
        painter.translate( g2w(_lastPos).x(), g2w(_lastPos).y() );
        painter.rotate( angle );
        painter.drawLine( QPoint(0,0), QPoint( 30,0 ) );

        painter.rotate( 30 );
        painter.drawLine( QPoint(0,0), QPoint( 30,0 ) );
    }

    painter.restore();
}

PointList LineDraw::anchorPoints()
{
    PointList res;
    res << g2w(_startPos) << g2w(_lastPos) << g2w(_startPos) + g2w((_lastPos - _startPos) / 2);
    return res;
}

Draw* LineDraw::clone()
{
    LineDraw* res = new LineDraw();
    *res = *this;
    return res;
}

QDomElement LineDraw::save( QDomDocument doc )
{
    QDomElement res = doc.createElement( QString::fromLatin1( "Line" ) );
    saveDrawAttr( &res );
    return res;
}

LineDraw::LineDraw( QDomElement elm )
    : Draw()
{
    readDrawAttr( elm );
}
