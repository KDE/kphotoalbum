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

#include "circledraw.h"
#include <qpainter.h>

void CircleDraw::draw( QPainter* painter, QMouseEvent* event )
{
    Draw::draw( painter, event );
    painter->drawEllipse( _startPos.x(), _startPos.y(), _lastPos.x()-_startPos.x(), _lastPos.y()-_startPos.y() );
}

PointList CircleDraw::anchorPoints()
{
    PointList res;
    QPoint center = _startPos + (_lastPos-_startPos)/2;
    res << QPoint( center.x(), _startPos.y() )
        << QPoint( center.x(), _lastPos.y() )
        << QPoint( _startPos.x(), center.y() )
        << QPoint( _lastPos.x(), center.y() );
    return res;
}

Draw* CircleDraw::clone()
{
    CircleDraw* res = new CircleDraw();
    *res = *this;
    return res;

}

QDomElement CircleDraw::save( QDomDocument doc )
{
    QDomElement res = doc.createElement( QString::fromLatin1( "Circle" ) );
    saveDrawAttr( &res );
    return res;
}

CircleDraw::CircleDraw( QDomElement elm )
    :Draw()
{
    readDrawAttr( elm );
}
