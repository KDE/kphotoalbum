#ifndef THUMBNAILVIEW_CELL_H
#define THUMBNAILVIEW_CELL_H

namespace ThumbnailView {

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

private:
    int _row;
    int _col;
};

}

#endif /* THUMBNAILVIEW_CELL_H */

