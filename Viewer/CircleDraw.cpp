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

#include "Viewer/CircleDraw.h"
#include <qpainter.h>

void Viewer::CircleDraw::draw( QPainter* painter, QMouseEvent* event )
{
    Draw::draw( painter, event );
    painter->drawEllipse( _startPos.x(), _startPos.y(), _lastPos.x()-_startPos.x(), _lastPos.y()-_startPos.y() );
}

PointList Viewer::CircleDraw::anchorPoints()
{
    PointList res;
    QPoint center = _startPos + (_lastPos-_startPos)/2;
    res << QPoint( center.x(), _startPos.y() )
        << QPoint( center.x(), _lastPos.y() )
        << QPoint( _startPos.x(), center.y() )
        << QPoint( _lastPos.x(), center.y() );
    return res;
}

Viewer::Draw* Viewer::CircleDraw::clone()
{
    Viewer::CircleDraw* res = new Viewer::CircleDraw();
    *res = *this;
    return res;

}

QDomElement Viewer::CircleDraw::save( QDomDocument doc )
{
    QDomElement res = doc.createElement( QString::fromLatin1( "Circle" ) );
    saveDrawAttr( &res );
    return res;
}

Viewer::CircleDraw::CircleDraw( QDomElement elm )
    :Draw()
{
    readDrawAttr( elm );
}
