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

#include "rectdraw.h"
#include <qpainter.h>

void RectDraw::draw( QPainter* painter, QMouseEvent* event )
{
    Draw::draw( painter, event );
    painter->drawRect( _startPos.x(), _startPos.y(), _lastPos.x()-_startPos.x(), _lastPos.y()-_startPos.y() );
}

PointList RectDraw::anchorPoints()
{
    PointList res;
    res << _startPos << _lastPos << QPoint( _startPos.x(), _lastPos.y() )
        << QPoint( _lastPos.x(), _startPos.y() );
    return res;
}

Draw* RectDraw::clone()
{
    RectDraw* res = new RectDraw();
    *res = *this;
    return res;

}

QDomElement RectDraw::save( QDomDocument doc )
{
    QDomElement res = doc.createElement( QString::fromLatin1( "Rectangle" ) );
    saveDrawAttr( &res );
    return res;
}

RectDraw::RectDraw( QDomElement elm )
    : Draw()
{
    readDrawAttr( elm );
}


