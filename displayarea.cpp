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
#include "draw.h"
#include "linedraw.h"
#include "rectdraw.h"
#include "circledraw.h"
#include <qpainter.h>
#include <qpicture.h>
#include "options.h"
#include "imageinfo.h"
#include "imagemanager.h"
#include "imageloader.h"
#include "viewhandler.h"
#include "drawhandler.h"
#include <qwmatrix.h>
#include <qlabel.h>

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
    bool block = _currentHandler->mousePressEvent( event );
    if ( !block )
        QWidget::mousePressEvent( event );
    update();
}

void DisplayArea::mouseMoveEvent( QMouseEvent* event )
{
    bool block = _currentHandler->mouseMoveEvent( event );
    if ( !block )
        QWidget::mousePressEvent( event );
    update();
}

void DisplayArea::mouseReleaseEvent( QMouseEvent* event )
{
    bool block = _currentHandler->mouseReleaseEvent( event );
    if ( !block )
        QWidget::mousePressEvent( event );
    update();
}

void DisplayArea::drawAll()
{
    if ( _loadedPixmap.isNull() )
        return;

    _drawingPixmap = scalePixmap(_loadedPixmap, width(), height() );
    if ( Options::instance()->showDrawings() )
         _drawHanler->drawAll( _drawingPixmap );
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
    ImageManager::instance()->load( info->fileName( false ), this, 0, -1,  -1, false, true );
}

void DisplayArea::pixmapLoaded( const QString&, int, int, int, const QImage& image )
{
    _loadedPixmap = image;
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
    return p;
}

void DisplayArea::paintEvent( QPaintEvent* )
{
    bitBlt( this, 0,0, &_viewPixmap );
}

QPixmap DisplayArea::scalePixmap( QPixmap pix, int width, int height )
{
    double pixWidth = pix.width();
    double pixHeight = pix.height();
    double ratio = width/pixWidth;

    if ( ratio * pixHeight > height ) {
        ratio = height/pixHeight;
        Q_ASSERT( ratio*pixWidth <= width );
    }

    QWMatrix matrix;
    matrix.scale( ratio, ratio );

    int ox = (int) ((width - pixWidth*ratio)/ratio)/2;
    int oy = (int) ((height - pixHeight*ratio)/ratio)/2;

    QPixmap res( width, height );
    res.fill( black );
    QPainter p(&res );
    p.setWorldMatrix( matrix );
    p.drawPixmap( ox, oy, pix );
    return res;
}

#include "displayarea.moc"
