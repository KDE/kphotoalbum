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

#include "draw.h"
#include <qevent.h>
#include <qwidget.h>
void Draw::startDraw( QMouseEvent* event )
{
    _startPos = event->pos();
    _lastPos = QPoint();
}

void Draw::draw( QPainter*, QMouseEvent* event )
{
    if ( event )
        _lastPos = event->pos();
}

void Draw::saveDrawAttr( QDomElement* elm )
{
    elm->setAttribute( QString::fromLatin1("_startPos.x"), _startPos.x() );
    elm->setAttribute( QString::fromLatin1("_startPos.y"), _startPos.y() );
    elm->setAttribute( QString::fromLatin1("_lastPos.x"), _lastPos.x() );
    elm->setAttribute( QString::fromLatin1("_lastPos.y"), _lastPos.y() );
}

void Draw::readDrawAttr( QDomElement elm )
{
    _startPos = QPoint( elm.attribute( QString::fromLatin1("_startPos.x"), QString::fromLatin1("0") ).toInt(),
                        elm.attribute( QString::fromLatin1("_startPos.y"), QString::fromLatin1("0") ).toInt() );
    _lastPos  = QPoint( elm.attribute( QString::fromLatin1("_lastPos.x"), QString::fromLatin1("0") ).toInt(),
                        elm.attribute( QString::fromLatin1("_lastPos.y"), QString::fromLatin1("0") ).toInt() );
}
