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


extern "C" {
#define XMD_H // prevent INT32 clash from jpeglib
#include <stdlib.h>
#include <stdio.h>
#include <jpeglib.h>
}

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
}

void DisplayArea::drawAll()
{
    if ( _croppedAndScaledImg.isNull() )
        return;

    QPixmap tmp = _croppedAndScaledImg;
    _drawingPixmap.resize( width(), height() );
    _drawingPixmap.fill( black );
    int x = ( width() - tmp.width() ) / 2;
    int y = ( height() - tmp.height() ) / 2;
    bitBlt( &_drawingPixmap, x, y, &tmp );

    if ( Options::instance()->showDrawings() && _drawHanler->hasDrawings() ) {
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
    // PENDING(blackie) Test if the image is JPG at all
    loadJPEG( &_loadedImage, info->fileName(false) );
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
    bitBlt( this, 0, 0, &_viewPixmap );
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
    cropAndScale();
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
    if ( _zEnd.x() > _loadedImage.width() )
        _zEnd.setX( _loadedImage.width() );
    if ( _zEnd.y() > _loadedImage.height() )
        _zEnd.setY( _loadedImage.height() );
    cropAndScale();
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

void DisplayArea::pan( const QPoint& p )
{
    // PENDING(blackie) Do boundary checks.
    _zStart += p;
    _zEnd += p;
    cropAndScale();
}

// Fudged Fast JPEG decoding code from GWENVIEW (picked out out digikam)

bool DisplayArea::loadJPEG(QImage* image, const QString& fileName )
{
    FILE* inputFile=fopen( fileName.latin1(), "rb");
    if(!inputFile) return false;

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, inputFile);
    jpeg_read_header(&cinfo, TRUE);

#if 0
    int size = li.width();
    int imgSize = QMAX(cinfo.image_width, cinfo.image_height);

    int scale=1;
    while(size*scale*2<=imgSize) {
        scale*=2;
    }
    if(scale>8) scale=8;

    cinfo.scale_num=1;
    cinfo.scale_denom=scale;
#endif
    // Create QImage
    jpeg_start_decompress(&cinfo);

    switch(cinfo.output_components) {
    case 3:
    case 4:
        image->create( cinfo.output_width, cinfo.output_height, 32 );
        break;
    case 1: // B&W image
        image->create( cinfo.output_width, cinfo.output_height, 8, 256 );
        for (int i=0; i<256; i++)
            image->setColor(i, qRgb(i,i,i));
        break;
    default:
        return false;
    }

    uchar** lines = image->jumpTable();
    while (cinfo.output_scanline < cinfo.output_height)
        jpeg_read_scanlines(&cinfo, lines + cinfo.output_scanline,
                            cinfo.output_height);
    jpeg_finish_decompress(&cinfo);

    // Expand 24->32 bpp
    if ( cinfo.output_components == 3 ) {
        for (uint j=0; j<cinfo.output_height; j++) {
            uchar *in = image->scanLine(j) + cinfo.output_width*3;
            QRgb *out = (QRgb*)( image->scanLine(j) );

            for (uint i=cinfo.output_width; i--; ) {
                in-=3;
                out[i] = qRgb(in[0], in[1], in[2]);
            }
        }
    }

    //int newMax = QMAX(cinfo.output_width, cinfo.output_height);
    //int newx = size*cinfo.output_width / newMax;
    //int newy = size*cinfo.output_height / newMax;

    //image=image.smoothScale( newx, newy);

    jpeg_destroy_decompress(&cinfo);
    fclose(inputFile);

    return true;
}


void DisplayArea::cropAndScale()
{
    if ( _zStart != QPoint(0,0) || _zEnd != QPoint( _loadedImage.width(), _loadedImage.height() ) )
        _croppedAndScaledImg = _loadedImage.copy( _zStart.x(), _zStart.y(), _zEnd.x() - _zStart.x(), _zEnd.y() - _zStart.y() );
    else
        _croppedAndScaledImg = _loadedImage;

    _croppedAndScaledImg = _croppedAndScaledImg.scale( width(), height(), QImage::ScaleMin );
    drawAll();
}

#include "displayarea.moc"
