/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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


#include "Viewer/DisplayArea.h"
#include <qpainter.h>
#include "options.h"
#include "imageinfo.h"
#include "Viewer/ViewHandler.h"
#include "Viewer/DrawHandler.h"
#include <qlabel.h>
#include "imagemanager.h"
#include <qcursor.h>
#include <qapplication.h>
#include <klocale.h>
#include <math.h>
#include "imagedb.h"

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
   either _viewHandler or _drawHandler.

   The code in the handlers should not care about actual zooming, therefore
   mouse coordinates are translated before they are given to the handlers
   in mouseMoveEvent etc.

   These handlers draw on _viewPixmap, but to do so, the painters need to
   be set up with transformation, as the pixmap is no longer the size of
   the original image, but rather the size of the display.



   To make viewing faster, preloading of images is done. However,
   preloading is not done of the full size images, as each image easily is
   approximate 20Mb large (5Mega pixels of each 4 bytes), which would fill
   the preloading cache with just a few images. Instead preloads of images
   in the view size are stored. This unfortunately complicated the code
   quite a bit.

   For the zooming and drawing to work, we need the total size of
   the images, even when it is scaled down in size. For that the preload
   info struct keeps this information, together with the image itself.

   Whenever zooming or rotation is performed, we need to load the real
   image, which is why we need the instance variable _cachedView and
   _reloadImageInProgress.

   To propagate the cache, we need to know which direction the
   images are viewed in, which is the job of the instance variable _forward.
*/

Viewer::DisplayArea::DisplayArea( QWidget* parent, const char* name )
    :QWidget( parent, name ), _info( 0 ), _reloadImageInProgress( false ), _forward(true), _curIndex(0),_busy( false )
{
    setBackgroundMode( NoBackground );

    _viewHandler = new ViewHandler( this );
    _drawHandler = new DrawHandler( this );
    _currentHandler = _viewHandler;
    _cache.setAutoDelete( true );

    connect( _drawHandler, SIGNAL( redraw() ), this, SLOT( drawAll() ) );

    // This is to ensure that people do see the drawing when they draw,
    // otherwise the drawing would disappear as soon as mouse was released.
    connect( _drawHandler, SIGNAL( active() ), this, SLOT( doShowDrawings() ) );
}

void Viewer::DisplayArea::mousePressEvent( QMouseEvent* event )
{
    QMouseEvent e( event->type(), mapPos( event->pos() ), event->button(), event->state() );
    double ratio;
    (void) offset( QABS( _zEnd.x()-_zStart.x() ), QABS( _zEnd.y()-_zStart.y() ), width(), height(), &ratio );
    bool block = _currentHandler->mousePressEvent( &e, event->pos(), ratio );
    if ( !block )
        QWidget::mousePressEvent( event );
    update();
}

void Viewer::DisplayArea::mouseMoveEvent( QMouseEvent* event )
{
    QMouseEvent e( event->type(), mapPos( event->pos() ), event->button(), event->state() );
    double ratio;
    (void) offset( QABS( _zEnd.x()-_zStart.x() ), QABS( _zEnd.y()-_zStart.y() ), width(), height(), &ratio );
    bool block = _currentHandler->mouseMoveEvent( &e, event->pos(), ratio );
    if ( !block )
        QWidget::mousePressEvent( event );
    update();
}

void Viewer::DisplayArea::mouseReleaseEvent( QMouseEvent* event )
{
    _cache.remove( _curIndex );
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

void Viewer::DisplayArea::drawAll()
{
    if ( _croppedAndScaledImg.isNull() )
        return;

    _drawingPixmap = _croppedAndScaledImg;

    if ( Options::instance()->showDrawings() && _drawHandler->hasDrawings() ) {
        QPainter painter( &_drawingPixmap );
        xformPainter( &painter );
        _drawHandler->drawAll( painter );
    }
    _viewPixmap = _drawingPixmap;
    repaint();
}

void Viewer::DisplayArea::startDrawing()
{
    _currentHandler = _drawHandler;
}

void Viewer::DisplayArea::stopDrawing()
{
    _drawHandler->stopDrawing();
    _currentHandler = _viewHandler;
    drawAll();
}

void Viewer::DisplayArea::toggleShowDrawings( bool b )
{
    Options::instance()->setShowDrawings( b );
    drawAll();
}

void Viewer::DisplayArea::setImage( ImageInfoPtr info, bool forward )
{
    _info = info;
    _loadedImage = QImage();

    // Find the index of the current image
    _curIndex = 0;
    for( QStringList::Iterator it = _imageList.begin(); it != _imageList.end(); ++it ) {
        if ( *it == info->fileName() )
            break;
        ++_curIndex;
    }

    ViewPreloadInfo* found = _cache[_curIndex];
    if ( found && found->angle == info->angle() ) {
        _loadedImage = found->img;
        _zStart = QPoint(0,0);
        _zEnd = QPoint( found->size.width(), found->size.height() );
        cropAndScale();
        _cachedView = true;
    }
    else {
        ImageRequest* request = new ImageRequest( info->fileName(), QSize( -1, -1 ), info->angle(), this );
        request->setPriority();
        ImageManager::instance()->load( request );
        busy();
        _cachedView = false;
    }
    _forward = forward;
    updatePreload();
}

void Viewer::DisplayArea::resizeEvent( QResizeEvent* )
{
    ImageManager::instance()->stop( this, ImageManager::StopOnlyNonPriorityLoads );
    _cache.fill(0); // Clear the cache
    if ( _info ) {
        cropAndScale();
        if ( _cachedView ) {
            ImageRequest* request = new ImageRequest( _info->fileName(),QSize(-1,-1), _info->angle(),  this );
            request->setPriority();
            ImageManager::instance()->load( request );
        }
    }
    updatePreload();
}

Viewer::DrawHandler* Viewer::DisplayArea::drawHandler()
{
    return _drawHandler;
}

QPainter* Viewer::DisplayArea::painter()
{
    _viewPixmap = _drawingPixmap;
    QPainter* p = new QPainter( &_viewPixmap );
    xformPainter( p );
    return p;
}

void Viewer::DisplayArea::paintEvent( QPaintEvent* )
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

QPoint Viewer::DisplayArea::offset( int logicalWidth, int logicalHeight, int physicalWidth, int physicalHeight, double* ratio )
{
    double rat = ((double)physicalWidth)/logicalWidth;

    if ( rat * logicalHeight > physicalHeight ) {
        rat = ((double)physicalHeight)/logicalHeight;
    }

    int ox = (int) (physicalWidth - logicalWidth*rat)/2;
    int oy = (int) (physicalHeight - logicalHeight*rat)/2;
    if ( ratio )
        *ratio = rat;
    return QPoint(ox,oy);
}



void Viewer::DisplayArea::zoom( QPoint p1, QPoint p2 )
{
    _cache.remove( _curIndex );
    normalize( p1, p2 );

    double ratio;
    QPoint off = offset( (p2-p1).x(), (p2-p1).y(), width(), height(), &ratio );
    off = off / ratio;

    if ( p1.x() - off.x() > 0 )
        p1.setX( p1.x() - off.x() );
    else
        p1.setX(0);

    int maxWidth;
    int maxHeight;
    if ( _cachedView ) {
        maxWidth = _zEnd.x();
        maxHeight = _zEnd.y();
    }
    else {
        maxWidth = _loadedImage.width();
        maxHeight = _loadedImage.height();
    }

    if ( p2.x() + off.x() < maxWidth )
        p2.setX( p2.x()+off.x() );
    else
        p2.setX( maxWidth );

    if ( p1.y() - off.y() > 0 )
        p1.setY( p1.y() - off.y() );
    else
        p1.setY(0);

    if ( p2.y() + off.y() < maxHeight )
        p2.setY( p2.y()+off.y() );
    else
        p2.setY( maxHeight );

    _zStart = p1;
    _zEnd = p2;
    if ( _cachedView ) {
        // This was a cached version, which means not full size, lets load
        // the real size now.
        ImageRequest* request = new ImageRequest( _info->fileName(), QSize(-1,-1), _info->angle(), this );
        request->setPriority();
        ImageManager::instance()->load( request );
        busy();
        _reloadImageInProgress = true;
    }
    else
        cropAndScale();
}

QPoint Viewer::DisplayArea::mapPos( QPoint p )
{
    QPoint off = offset( QABS( _zEnd.x()-_zStart.x() ), QABS( _zEnd.y()-_zStart.y() ), width(), height(), 0 );
    p -= off;
    int x = (int) (_zStart.x() + (_zEnd.x()-_zStart.x())*((double)p.x()/ (width()-2*off.x())));
    int y = (int) (_zStart.y() + (_zEnd.y()-_zStart.y())*((double)p.y()/ (height()-2*off.y())));

    return QPoint( x, y );

}

void Viewer::DisplayArea::xformPainter( QPainter* p )
{
    QPoint off = offset( QABS( _zEnd.x()-_zStart.x() ), QABS( _zEnd.y()-_zStart.y() ), width(), height(), 0 );
    double s = (width()-2*off.x())/QABS( (double)_zEnd.x()-_zStart.x());
    p->scale( s, s );
    p->translate( -_zStart.x(), -_zStart.y() );
}

void Viewer::DisplayArea::zoomIn()
{
    QPoint size = (_zEnd-_zStart);
    QPoint p1 = _zStart + size*(0.2/2);
    QPoint p2 = _zEnd - size*(0.2/2);
    zoom(p1, p2);
}

void Viewer::DisplayArea::zoomOut()
{
    if ( _zStart == QPoint(0,0) && _zEnd == QPoint( _loadedImage.width(), _loadedImage.height() ) )
        return; // Bail out if we have zoomed all the way out, to avoid spending time in scaling

    QPoint size = (_zEnd-_zStart);
    QPoint p1 = _zStart - size*(0.25/2);
    QPoint p2 = _zEnd + size*(0.25/2);
    zoom(p1,p2);
}

void Viewer::DisplayArea::zoomFull()
{
    if ( !_cachedView ) // Cached views are always full views
        zoom( QPoint(0,0), QPoint( _loadedImage.width(), _loadedImage.height() ) );
}


void Viewer::DisplayArea::normalize( QPoint& p1, QPoint& p2 )
{
    int minx = QMIN( p1.x(), p2.x() );
    int miny = QMIN( p1.y(), p2.y() );
    int maxx = QMAX( p1.x(), p2.x() );
    int maxy = QMAX( p1.y(), p2.y() );
    p1 = QPoint( minx, miny );
    p2 = QPoint( maxx, maxy );
}

void Viewer::DisplayArea::pan( const QPoint& point )
{
    if ( _cachedView )
        return; // Cached views are always full screen, so no panning is available.

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

void Viewer::DisplayArea::cropAndScale()
{
    ViewPreloadInfo* info = _cache[_curIndex];

    if ( info && info->angle == _info->angle() ) {
        _croppedAndScaledImg = info->img;
        _zEnd = QPoint( info->size.width(), info->size.height() );
        _cachedView = true;
    }
    else {
        if ( _loadedImage.isNull() || _cachedView ) {
            return;
        }

        if ( _zStart != QPoint(0,0) || _zEnd != QPoint( _loadedImage.width(), _loadedImage.height() ) ) {
            _croppedAndScaledImg = _loadedImage.copy( _zStart.x(), _zStart.y(), _zEnd.x() - _zStart.x(), _zEnd.y() - _zStart.y() );
        }
        else
            _croppedAndScaledImg = _loadedImage;

        if ( !_croppedAndScaledImg.isNull() ) // I don't know how this can happen, but it seems not to be dangerous.
            _croppedAndScaledImg = _croppedAndScaledImg.smoothScale( width(), height(), QImage::ScaleMin );
    }

    drawAll();
}

void Viewer::DisplayArea::doShowDrawings()
{
    Options::instance()->setShowDrawings( true );
}

QImage Viewer::DisplayArea::currentViewAsThumbnail() const
{
    if ( _croppedAndScaledImg.isNull() )
        return QImage();
    else
        return _croppedAndScaledImg.smoothScale( 128, 128, QImage::ScaleMin );
}

void Viewer::DisplayArea::pixmapLoaded( const QString& fileName, const QSize& imgSize, const QSize& fullSize, int angle, const QImage& img, bool loadedOK )
{
    if ( loadedOK && fileName == _info->fileName() ) {
        _loadedImage = img;
        _cachedView = !( imgSize == fullSize || imgSize == QSize(-1,-1) );
        if ( !_reloadImageInProgress ) {
            _zStart=QPoint( 0, 0 );
            _zEnd=QPoint( img.width(), img.height() );
        }
        else
            _reloadImageInProgress = false;

        cropAndScale();
    }
    else {
        if ( imgSize != size() )
            return; // Might be an old preload version, or a loaded version that never made it in time

        ViewPreloadInfo* info = new ViewPreloadInfo( img, fullSize, angle );
        bool ok = _cache.insert( indexOf(fileName), info );
        if ( !ok )
            delete info;
        updatePreload();
    }
    unbusy();
    emit possibleChange();
}

void Viewer::DisplayArea::setImageList( const QStringList& list )
{
    _imageList = list;
    _cache.fill( 0, list.count() );
}

void Viewer::DisplayArea::updatePreload()
{
    uint cacheSize = ( Options::instance()->viewerCacheSize() * 1024 * 1024 ) / (width()*height()*4);
    bool cacheFull = (_cache.count() > cacheSize);

    int incr = ( _forward ? 1 : -1 );
    int nextOnesInCache = 0;
    // Iterate from the current image in the direction of the viewing
    for ( int i = _curIndex+incr; true ; i += incr ) {
        if ( _forward ? ( i >= (int) _imageList.count() ) : (i < 0) )
            break;

        ImageInfoPtr info = ImageDB::instance()->info(_imageList[i]);
        if ( !info ) {
            qWarning("Info was null for index %d!", i);
            return;
        }

        if ( _cache[i] ) {
            nextOnesInCache++;
            if ( nextOnesInCache >= ceil(cacheSize/2.0) && cacheFull ) {
                // Ok enough images in cache
                return;
            }
        }
        else {
            ImageRequest* request = new ImageRequest( info->fileName(), QSize(width(), height()), info->angle(), this );
            ImageManager::instance()->load( request );

            if ( cacheFull ) {
                // The cache was full, we need to delete an item from the cache.

                // First try to find an item from the direction we came from
                for ( int j = ( _forward ? 0 : _imageList.count() -1 );
                      j != _curIndex;
                      j += ( _forward ? 1 : -1 ) ) {
                    if ( _cache[j] ) {
                        _cache.remove(j);
                        return;
                    }
                }

                // OK We found no item in the direction we came from (think of home/end keys)
                for ( int j = ( _forward ? _imageList.count() -1 : 0 );
                      j != _curIndex;
                      j += ( _forward ? -1 : 1 ) ) {
                    if ( _cache[j] ) {
                        _cache.remove(j);
                        return;
                    }
                }

                Q_ASSERT( false ); // We should never get here.
            }

            return;
        }
    }
}


int Viewer::DisplayArea::indexOf( const QString& fileName )
{
    int i = 0;
    for( QStringList::ConstIterator it = _imageList.begin(); it != _imageList.end(); ++it ) {
        if ( *it == fileName )
            break;
        ++i;
    }
    return i;
}

void Viewer::DisplayArea::busy()
{
    if ( !_busy )
        qApp->setOverrideCursor( Qt::WaitCursor );
    _busy = true;
}

void Viewer::DisplayArea::unbusy()
{
    if ( _busy )
        qApp->restoreOverrideCursor();
    _busy = false;
}

#include "DisplayArea.moc"
