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

