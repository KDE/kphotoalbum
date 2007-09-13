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
#ifndef THUMBNAILVIEW_CELL_H
#define THUMBNAILVIEW_CELL_H

namespace ThumbnailView {
enum CoordinateSystem {ViewportCoordinates, ContentsCoordinates };


class Cell
{
public:
    Cell() : _row(0), _col(0) {}
    Cell( int row, int col ) : _row( row ), _col( col ) {}
    int row() const { return _row; }
    int col() const { return _col; }
    int& row() { return _row; }
    int& col() { return _col; }
    bool operator>( const Cell& other )
    {
        return _row > other._row || ( _row == other._row && _col > other._col  );
    }
    bool operator<( const Cell& other )
    {
        return _row < other._row || ( _row == other._row && _col < other._col  );
    }
    bool operator==( const Cell& other )
    {
        return _row == other._row && _col == other._col;
    }

    static const Cell& invalidCell()
    {
        static Cell x(-1, -1);
        return x;
    }

private:
    int _row;
    int _col;
};

}

#endif /* THUMBNAILVIEW_CELL_H */

