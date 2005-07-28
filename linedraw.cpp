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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "linedraw.h"
#include <qpainter.h>
#include <qevent.h>
#include <math.h>

void LineDraw::draw( QPainter* painter, QMouseEvent* event )
{
    Draw::draw( painter, event );

    painter->save();
    painter->drawLine( _startPos, _lastPos );

    QPoint diff = QPoint( _lastPos.x() - _startPos.x(), _lastPos.y() - _startPos.y() );
    double dx = diff.x();
    double dy = diff.y();

    if ( dx != 0 || dy != 0 ) {
        if( dy < 0 ) dx = -dx;
        double angle = acos(dx/sqrt( dx*dx+dy*dy ))*180./M_PI;
        if( dy < 0 ) angle += 180.;

        // angle is now the angle of the line.

        angle = angle + 180 - 15;
        painter->translate( _lastPos.x(), _lastPos.y() );
        painter->rotate( angle );
        painter->drawLine( QPoint(0,0), QPoint( 30,0 ) );

        painter->rotate( 30 );
        painter->drawLine( QPoint(0,0), QPoint( 30,0 ) );
    }

    painter->restore();
}

PointList LineDraw::anchorPoints()
{
    PointList res;
    res << _startPos << _lastPos << _startPos + (_lastPos - _startPos) / 2;
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
