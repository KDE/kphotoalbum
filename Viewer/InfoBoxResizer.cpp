/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "InfoBoxResizer.h"
#include <QDebug>
#include "InfoBox.h"

Viewer::InfoBoxResizer::InfoBoxResizer( Viewer::InfoBox* infoBox )
    :_infoBox( infoBox )
{
}

void Viewer::InfoBoxResizer::setPos(  QPoint pos )
{
    QRect rect = _infoBox->geometry();
    pos = _infoBox->mapToParent( pos );

    if ( _left )
        rect.setLeft( pos.x() );
    if (_right)
        rect.setRight(pos.x());
    if ( _top )
        rect.setTop( pos.y() );
    if ( _bottom )
        rect.setBottom( pos.y() );

    if ( rect.width() > 100 && rect.height() > 50 )
        _infoBox->setGeometry( rect );
}

void Viewer::InfoBoxResizer::setup( bool left, bool right, bool top, bool bottom )
{
    _left = left;
    _right = right;
    _top = top;
    _bottom = bottom;
    _active = true;
}

void Viewer::InfoBoxResizer::deactivate()
{
    _active = false;
}

bool Viewer::InfoBoxResizer::isActive() const
{
    return _active;
}
// vi:expandtab:tabstop=4 shiftwidth=4:
