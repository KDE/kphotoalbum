/*
 *  Copyright (c) 2003-2004 Jesper K. Pedersen <blackie@kde.org>
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
#include "viewhandler.h"
#include "drawhandler.h"
#include <qlabel.h>

/**
   Area displaying the actual image in the viewer.

   The purpose of this class is to display the actual image in the
   viewer. This involves controlling zooming and drawing on the images.

   This class is quite complicated as it had to both be fast and memory
   efficient. The following are dead end tried:
   1) Initially QPainter::setWindow was used for zooming the images, but
      this had the effect that if you zoom to 100x100 from a 2300x1700
      image on a 800x600 display, then Qt would internally create a pixmap
      with the size (2300/100)*800, (1700/100)*600, which takes up 1.4Gb of
      memory!
   2) I tried doing all scaling and cropping using QPixmap's as that would
      allow me to keep all transformations on the X Server site (making
      resizing fast - or I beleived so). Unfortunately it showed up that
      this was much slower than doing it using QImage, and the result was
      thus that the looking at a series of images was slow.

   The process is as follows:
   - The image loaded from disk is rotated and stored in _loadedImage. This
     image is as large as the image on disk.
   - Then _loadedImage is cropped and scaled to _croppedAndScaledImg. This
     image is the size of the display. Resizing the window thus needs to
     start from this step.
   - Then _croppedAndScaledImg is converted to _drawingPixmap. Completed
     drawings are drawn into _loadedPixmap
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

    // This is to ensure that people do see the drawing when they draw,
    // otherwise the drawing would disappear as soon as mouse was released.
    connect( _drawHanler, SIGNAL( active() ), this, SLOT( doShowDrawings() ) );
}

void DisplayArea::mousePressEvent( QMouseEvent* event )
{
    QMouseEvent e( event->type(), mapPos( event->pos() ), event->button(), event->state() );
    double ratio;
    (void) offset( QABS( _zEnd.x()-_zStart.x() ), QABS( _zEnd.y()-_zStart.y() ), width(), height(), &ratio );
    bool block = _currentHandler->mousePressEvent( &e, event->pos(), ratio );
    if ( !block )
        QWidget::mousePressEvent( event );
    update();
}

void DisplayArea::mouseMoveEvent( QMouseEvent* event )
{
    QMouseEvent e( event->type(), mapPos( event->pos() ), event->button(), event->state() );
    double ratio;
    (void) offset( QABS( _zEnd.x()-_zStart.x() ), QABS( _zEnd.y()-_zStart.y() ), width(), height(), &ratio );
    bool block = _currentHandler->mouseMoveEvent( &e, event->pos(), ratio );
    if ( !block )
        QWidget::mousePressEvent( event );
    update();
}

void DisplayArea::mouseReleaseEvent( QMouseEvent* event )
{
    QMouseEvent e( event->type(), mapPos( event->pos() ), event->button(), event->state() );
    double ratio;
    (void) offset( QABS( _zEnd.x()-_zStart.x() ), QABS( _zEnd.y()-_zStart.y() ), width(), height(), &ratio );
    bool block = _currentHandler->mouseReleaseEvent( &e, event->pos(), ratio );
    if ( !block ) {
        QWidget::mouseReleaseEvent( event );
    }
    emit possibleChange();
    drawAll();
}

void DisplayArea::drawAll()
{
    if ( _croppedAndScaledImg.isNull() )
        return;

    _drawingPixmap = _croppedAndScaledImg;

    if ( Options::instance()->showDrawings() && _drawHanler->hasDrawings() ) {
        QPainter painter( &_drawingPixmap );
        xformPainter( &painter );
        _drawHanler->drawAll( painter );
    }
    _viewPixmap = _drawingPixmap;
    repaint();
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
    _loadedImage = info->load();

    _zStart = QPoint(0,0);
    _zEnd = QPoint( _loadedImage.width(), _loadedImage.height() );
    cropAndScale();
}

void DisplayArea::resizeEvent( QResizeEvent* )
{
    cropAndScale();
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
    int x = ( width() - _viewPixmap.width() ) / 2;
    int y = ( height() - _viewPixmap.height() ) / 2;
    bitBlt( this, x, y, &_viewPixmap );
    QPainter p( this );
    p.fillRect( 0,0, width(), y, black ); // top
    p.fillRect( 0,height()-y, width(), height()-y, black ); // bottom
    p.fillRect( 0,0, x, height(), black ); // left
    p.fillRect( width()-x, 0, width()-x, height(), black ); // right
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

    double ratio;
    QPoint off = offset( (p2-p1).x(), (p2-p1).y(), width(), height(), &ratio );
    off = off / ratio;

    if ( p1.x() - off.x() > 0 )
        p1.setX( p1.x() - off.x() );
    else
        p1.setX(0);

    if ( p2.x() + off.x() < _loadedImage.width() )
        p2.setX( p2.x()+off.x() );
    else
        p2.setX( _loadedImage.width() );

    if ( p1.y() - off.y() > 0 )
        p1.setY( p1.y() - off.y() );
    else
        p1.setY(0);

    if ( p2.y() + off.y() < _loadedImage.height() )
        p2.setY( p2.y()+off.y() );
    else
        p2.setY( _loadedImage.height() );

    _zStart = p1;
    _zEnd = p2;
    cropAndScale();
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
    double s = (width()-2*off.x())/QABS( (double)_zEnd.x()-_zStart.x());
    p->scale( s, s );
    p->translate( -_zStart.x(), -_zStart.y() );
}

void DisplayArea::zoomIn()
{
    QPoint size = (_zEnd-_zStart);
    QPoint p1 = _zStart + size*(0.2/2);
    QPoint p2 = _zEnd - size*(0.2/2);
    zoom(p1, p2);
}

void DisplayArea::zoomOut()
{
    QPoint size = (_zEnd-_zStart);
    QPoint p1 = _zStart - size*(0.25/2);
    QPoint p2 = _zEnd + size*(0.25/2);
    zoom(p1,p2);
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

void DisplayArea::pan( const QPoint& point )
{
    QPoint p = point;
    if ( p.x() < 0 && _zStart.x() < -p.x() )
        p.setX( -_zStart.x() );

    if ( p.y() < 0 && _zStart.y() < -p.y() )
        p.setY( -_zStart.y() );

    if ( p.x() > 0 && p.x() + _zEnd.x() > _loadedImage.width() )
        p.setX( _loadedImage.width() - _zEnd.x() );

    if ( p.y() > 0 && p.y() + _zEnd.y() > _loadedImage.height() )
        p.setY( _loadedImage.height() - _zEnd.y() );

    _zStart += p;
    _zEnd += p;
    cropAndScale();
}

void DisplayArea::cropAndScale()
{
    if ( _loadedImage.isNull() )
        return;

    if ( _zStart != QPoint(0,0) || _zEnd != QPoint( _loadedImage.width(), _loadedImage.height() ) )
        _croppedAndScaledImg = _loadedImage.copy( _zStart.x(), _zStart.y(), _zEnd.x() - _zStart.x(), _zEnd.y() - _zStart.y() );
    else
        _croppedAndScaledImg = _loadedImage;

    _croppedAndScaledImg = _croppedAndScaledImg.smoothScale( width(), height(), QImage::ScaleMin );

    drawAll();
}

void DisplayArea::doShowDrawings()
{
    Options::instance()->setShowDrawings( true );
}

QImage DisplayArea::currentViewAsThumbnail() const
{
    return _croppedAndScaledImg.smoothScale( 128, 128, QImage::ScaleMin );
}

#include "displayarea.moc"
