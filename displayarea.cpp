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

#include "displayarea.h"
#include <qpainter.h>
#include "options.h"
#include "imageinfo.h"
#include "imagemanager.h"
#include "viewhandler.h"
#include "drawhandler.h"
#include <qwmatrix.h>
#include <qlabel.h>

/**
   Area displaying the actual image in the viewer.

   The purpose of this class is to display the actual image in the
   viewer. This involves controlling zooming and drawing on the images.

   This class is quite complicated as it had to both be fast and memory
   efficient. This had the following consequence:
   1) Images must be handled as pixmap all the time, that is no
      transformation to QImage is allowed, as the transformation would
      result in the image being copied from the XServer to the kimdaba
      process.
   2) Images should be scaled down to the size of the widget as soon as
      possible. Images are easily 2300x1700 pixels (4 mega pixel), while
      they often only are displayed on a 800x600 display. (Handling
      transformations using the later saves a factor 8 in memory)
   3) Initially QPainter::setWindow was used for zooming the images, but
      this had the effect that if you zoom to 100x100 from a 2300x1700
      image on a 800x600 display, then Qt would internally create a pixmap
      with the size (2300/100)*800, (1700/100)*600, which takes up 1.4Gb of
      memory!

   The process is as follows:
   - The image loaded from disk is converted and stored in
     _loadedPixmap. This pixmap is as large as the image on disk.
   - Then _loadedPixmap is converted to _drawingPixmap. (This pixmap is the
     size of the display). Completed drawings are drawn into _loadedPixmap
   - When the user draws a new shape, then for each mouse movement
     _loadedPixmap is copied to _viewPixmap, in which the drawing are made.
   - Finally in paintEvent _viewPixmap is bitBlt'ed to the screen.

   The scaling process and the drawing process is both implemented by
   overriding mouse events. To make the code more readable, the strategy
   pattern is used to separate the two, and when the widget sees a
   mousePress, mouseMove or mouseRelease event then it delegates this to
   either _viewHandler or _drawHanler.

   The code in the handlers should not care about actual zooming, therefore
   mouse coordinates are translated before they are given to the handlers
   in mouseMoveEvent etc.

   These handlers draw on _viewPixmap, but to do so, the painters need to
   be set up with transformation, as the pixmap is no longer the size of
   the original image, but rather the size of the display.
*/

DisplayArea::DisplayArea( QWidget* parent, const char* name )
    :QWidget( parent, name )
{
    setBackgroundMode( NoBackground );

    _viewHandler = new ViewHandler( this );
    _drawHanler = new DrawHandler( this );
    _currentHandler = _viewHandler;

    connect( _drawHanler, SIGNAL( redraw() ), this, SLOT( drawAll() ) );
}

void DisplayArea::mousePressEvent( QMouseEvent* event )
{
    QMouseEvent e( event->type(), mapPos( event->pos() ), event->button(), event->state() );
    bool block = _currentHandler->mousePressEvent( &e  );
    if ( !block )
        QWidget::mousePressEvent( event );
    update();
}

void DisplayArea::mouseMoveEvent( QMouseEvent* event )
{
    QMouseEvent e( event->type(), mapPos( event->pos() ), event->button(), event->state() );
    bool block = _currentHandler->mouseMoveEvent( &e );
    if ( !block )
        QWidget::mousePressEvent( event );
    update();
}

void DisplayArea::mouseReleaseEvent( QMouseEvent* event )
{
    QMouseEvent e( event->type(), mapPos( event->pos() ), event->button(), event->state() );
    bool block = _currentHandler->mouseReleaseEvent( &e );
    if ( !block )
        QWidget::mousePressEvent( event );
    drawAll();
    update();
}

void DisplayArea::drawAll()
{
    if ( _loadedPixmap.isNull() )
        return;

    QPixmap tmp( QABS( _zEnd.x()-_zStart.x() ), QABS( _zEnd.y()-_zStart.y() ) );
    bitBlt( &tmp, QPoint(0,0), &_loadedPixmap, QRect( _zStart, _zEnd ) );
    _drawingPixmap = scalePixmap( tmp, width(), height() );

    if ( Options::instance()->showDrawings() ) {
        QPainter painter( &_drawingPixmap );
        xformPainter( &painter );
        _drawHanler->drawAll( painter );
    }
    _viewPixmap = _drawingPixmap;
    update();
}

void DisplayArea::startDrawing()
{
    _currentHandler = _drawHanler;
}

void DisplayArea::stopDrawing()
{
    _drawHanler->stopDrawing();
    _currentHandler = _viewHandler;
    drawAll();
}

void DisplayArea::toggleShowDrawings( bool b )
{
    Options::instance()->setShowDrawings( b );
    drawAll();
}

void DisplayArea::setImage( ImageInfo* info )
{
    _info = info;
    ImageManager::instance()->load( info->fileName( false ), this, info->angle(), -1,  -1, false, true );
}

void DisplayArea::pixmapLoaded( const QString&, int, int, int, const QImage& image )
{
    _loadedPixmap = image;
    _zStart = QPoint(0,0);
    _zEnd = QPoint( image.width(), image.height() );
    drawAll();
    update();
}

void DisplayArea::resizeEvent( QResizeEvent* )
{
    drawAll();
}

DrawHandler* DisplayArea::drawHandler()
{
    return _drawHanler;
}

QPainter* DisplayArea::painter()
{
    _viewPixmap = _drawingPixmap;
    QPainter* p = new QPainter( &_viewPixmap );
    xformPainter( p );
    return p;
}

void DisplayArea::paintEvent( QPaintEvent* )
{
    if ( _viewPixmap.isNull() )
        return;

    QPainter p(this);
    p.drawPixmap( 0,0, _viewPixmap );
}

QPixmap DisplayArea::scalePixmap( QPixmap pix, int width, int height )
{
    int pixWidth = pix.width();
    int pixHeight = pix.height();
    double ratio;
    QPoint off = offset(pixWidth, pixHeight, width, height, &ratio );
    off = off/ratio;

    QWMatrix matrix;
    matrix.scale( ratio, ratio );

    QPixmap res( width, height );
    res.fill( black );
    QPainter p(&res );
    p.setWorldMatrix( matrix );
    p.drawPixmap( off, pix );
    return res;
}

QPoint DisplayArea::offset( int logicalWidth, int logicalHeight, int physicalWidth, int physicalHeight, double* ratio )
{
    double rat = ((double)physicalWidth)/logicalWidth;

    if ( rat * logicalHeight > physicalHeight ) {
        rat = ((double)physicalHeight)/logicalHeight;
        Q_ASSERT( rat * logicalWidth <= physicalWidth );
    }

    int ox = (int) (physicalWidth - logicalWidth*rat)/2;
    int oy = (int) (physicalHeight - logicalHeight*rat)/2;
    if ( ratio )
        *ratio = rat;
    return QPoint(ox,oy);
}



void DisplayArea::zoom( QPoint p1, QPoint p2 )
{
    normalize( p1, p2 );
    _zStart = p1;
    _zEnd = p2;
}

QPoint DisplayArea::mapPos( QPoint p )
{
    QPoint off = offset( QABS( _zEnd.x()-_zStart.x() ), QABS( _zEnd.y()-_zStart.y() ), width(), height(), 0 );

    p -= off;
    int x = (int) (_zStart.x() + (_zEnd.x()-_zStart.x())*((double)p.x()/ (width()-2*off.x())));
    int y = (int) (_zStart.y() + (_zEnd.y()-_zStart.y())*((double)p.y()/ (height()-2*off.y())));
    return QPoint( x, y );
}

void DisplayArea::xformPainter( QPainter* p )
{
    QPoint off = offset( QABS( _zEnd.x()-_zStart.x() ), QABS( _zEnd.y()-_zStart.y() ), width(), height(), 0 );

    p->translate( off.x(), off.y() );
    double s = (width()-2*off.x())/QABS( (double)_zEnd.x()-_zStart.x());
    p->scale( s, s );
    p->translate( -_zStart.x(), -_zStart.y() );
}

void DisplayArea::zoomIn()
{
    QPoint size = (_zEnd-_zStart);
    _zStart += size*(0.5/2);
    _zEnd -= size*(0.5/2);
    drawAll();
}

void DisplayArea::zoomOut()
{
    QPoint size = (_zEnd-_zStart);
    _zStart -= size*(1.0/2);
    if ( _zStart.x() < 0 )
        _zStart.setX(0);
    if ( _zStart.y() < 0 )
        _zStart.setY(0);

    _zEnd += size*(1.0/2);
    if ( _zEnd.x() > _loadedPixmap.width() )
        _zEnd.setX( _loadedPixmap.width() );
    if ( _zEnd.y() > _loadedPixmap.height() )
        _zEnd.setY( _loadedPixmap.height() );
    drawAll();
}

void DisplayArea::normalize( QPoint& p1, QPoint& p2 )
{
    int minx = QMIN( p1.x(), p2.x() );
    int miny = QMIN( p1.y(), p2.y() );
    int maxx = QMAX( p1.x(), p2.x() );
    int maxy = QMAX( p1.y(), p2.y() );
    p1 = QPoint( minx, miny );
    p2 = QPoint( maxx, maxy );
}

#include "displayarea.moc"
