/* Copyright (C) 2003-2009 Jesper K. Pedersen <blackie@kde.org>

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
#ifndef CELLGEOMETRY_H
#define CELLGEOMETRY_H
#include "ThumbnailComponent.h"
#include <QMap>

class QRect;
class QSize;

namespace DB { class ResultId; }

namespace ThumbnailView
{
class Cell;
class ThumbnailFactory;

class CellGeometry :public ThumbnailComponent
{
public:
    CellGeometry( ThumbnailFactory* factory );
    QSize cellSize() const;
    QRect iconGeometry( int row, int col ) const;
    int textHeight( int charHeight, bool reCalc ) const;
};

}


#endif /* CELLGEOMETRY_H */

