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

#ifndef DISPLAYAREA_H
#define DISPLAYAREA_H
#include <qlabel.h>
#include <qpixmap.h>
#include <qptrlist.h>
#include "drawlist.h"
#include "imageclient.h"
#include <qimage.h>
class Draw;
class ImageInfo;
class ViewHandler_viewHandler;
class DrawHandler;
class DisplayAreaHandler;
class ViewHandler;

class DisplayArea :public QWidget,  public ImageClient {
Q_OBJECT
public:
    DisplayArea( QWidget* parent, const char* name = 0 );
    void startDrawing();
    void stopDrawing();
    void setImage( ImageInfo* info );
    DrawHandler* drawHandler();

public slots:
    void toggleShowDrawings( bool );
    void zoomIn();
    void zoomOut();

protected slots:
    void drawAll();

protected:
    virtual void mousePressEvent( QMouseEvent* event );
    virtual void mouseMoveEvent( QMouseEvent* event );
    virtual void mouseReleaseEvent( QMouseEvent* event );
    virtual void resizeEvent( QResizeEvent* event );
    virtual void paintEvent( QPaintEvent* event );
    void pixmapLoaded( const QString&, int, int, int, const QImage& image );
    QPixmap scalePixmap( QPixmap pix, int width, int height );
    QPoint mapPos( QPoint );
    QPoint offset( int logicalWidth, int logicalHeight, int physicalWidth, int physicalHeight, double* ratio );
    void xformPainter( QPainter* );

    friend class DrawHandler;
    friend class ViewHandler;
    QPainter* painter();
    void zoom( QPoint p1, QPoint p2 );
    void normalize( QPoint& p1, QPoint& p2 );


private:
    QPixmap _loadedPixmap;
    QPixmap _drawingPixmap;
    QPixmap _viewPixmap;
    ImageInfo* _info;

    ViewHandler* _viewHandler;
    DrawHandler* _drawHanler;
    DisplayAreaHandler* _currentHandler;

    QPoint _zStart; // Stands for zoom start
    QPoint _zEnd;
};


#endif /* DISPLAYAREA_H */

