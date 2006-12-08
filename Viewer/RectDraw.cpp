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

#include "Viewer/RectDraw.h"
#include <qpainter.h>

void Viewer::RectDraw::draw( QPainter* painter, QMouseEvent* event )
{
    Draw::draw( painter, event );
    painter->drawRect( _startPos.x(), _startPos.y(), _lastPos.x()-_startPos.x(), _lastPos.y()-_startPos.y() );
}

PointList Viewer::RectDraw::anchorPoints()
{
    PointList res;
    res << _startPos << _lastPos << QPoint( _startPos.x(), _lastPos.y() )
        << QPoint( _lastPos.x(), _startPos.y() );
    return res;
}

Viewer::Draw* Viewer::RectDraw::clone()
{
    Viewer::RectDraw* res = new Viewer::RectDraw();
    *res = *this;
    return res;

}

QDomElement Viewer::RectDraw::save( QDomDocument doc )
{
    QDomElement res = doc.createElement( QString::fromLatin1( "Rectangle" ) );
    saveDrawAttr( &res );
    return res;
}

Viewer::RectDraw::RectDraw( QDomElement elm )
    : Draw()
{
    readDrawAttr( elm );
}


