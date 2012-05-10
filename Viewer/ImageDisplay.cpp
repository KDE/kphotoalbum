/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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


#include "ImageDisplay.h"
#include <qpainter.h>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <KMessageBox>
#include <klocale.h>
#include "Settings/SettingsData.h"
#include "Viewer/ViewHandler.h"
#include "ImageManager/AsyncLoader.h"
#include <qcursor.h>
#include <qapplication.h>
#include <math.h>
#include "DB/ImageDB.h"
#include <qtimer.h>

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
   - The image loaded from disk is rotated and stored in _loadedImage.
     Initially this image is as large as the view, until the
     user starts zooming, at which time the image is reloaded to the size
     as it is on disk.
   - Then _loadedImage is cropped and scaled to _croppedAndScaledImg. This
     image is the size of the display. Resizing the window thus needs to
     redo step.
   - Finally in paintEvent _croppedAndScaledImg is drawn to the screen.

   The aboce might very likely be simplified. Back in the old days it needed to be that
   complex to allow drawing on images.

   To propagate the cache, we need to know which direction the
   images are viewed in, which is the job of the instance variable _forward.
*/

Viewer::ImageDisplay::ImageDisplay( QWidget* parent)
    :AbstractDisplay( parent ), _reloadImageInProgress( false ), _forward(true), _curIndex(0),_busy( false ),
    _cursorHiding(true)
{
    _viewHandler = new ViewHandler( this );
    _cache.setAutoDelete( true );

    setMouseTracking( true );
    _cursorTimer = new QTimer( this );
    _cursorTimer->setSingleShot(true);
    connect( _cursorTimer, SIGNAL( timeout() ), this, SLOT( hideCursor() ) );
    showCursor();

}

/**
 * If mouse cursor hiding is enabled, hide the cursor right now
 */
void Viewer::ImageDisplay::hideCursor() {
    if (_cursorHiding)
        setCursor( Qt::blankCursor );
}

/**
 * If mouse cursor hiding is enabled, show normal cursor and start a timer that will hide it later
 */
void Viewer::ImageDisplay::showCursor() {
    if (_cursorHiding) {
        unsetCursor();
        _cursorTimer->start( 1500 );
    }
}

/**
 * Prevent hideCursor() and showCursor() from altering cursor state
 */
void Viewer::ImageDisplay::disableCursorHiding() {
    _cursorHiding = false;
}

/**
 * Enable automatic mouse cursor hiding
 */
void Viewer::ImageDisplay::enableCursorHiding() {
    _cursorHiding = true;
}

void Viewer::ImageDisplay::mousePressEvent( QMouseEvent* event )
{
    // disable cursor hiding till button release
    disableCursorHiding();

    QMouseEvent e( event->type(), mapPos( event->pos() ), event->button(), event->buttons(), event->modifiers() );
    double ratio = sizeRatio( QSize(_zEnd.x()-_zStart.x(), _zEnd.y()-_zStart.y()), size() );
    bool block = _viewHandler->mousePressEvent( &e, event->pos(), ratio );
    if ( !block )
        QWidget::mousePressEvent( event );
    update();
}

void Viewer::ImageDisplay::mouseMoveEvent( QMouseEvent* event )
{
    // just reset the timer
    showCursor();

    QMouseEvent e( event->type(), mapPos( event->pos() ), event->button(), event->buttons(), event->modifiers() );
    double ratio = sizeRatio( QSize(_zEnd.x()-_zStart.x(), _zEnd.y()-_zStart.y()), size() );
    bool block = _viewHandler->mouseMoveEvent( &e, event->pos(), ratio );
    if ( !block )
        QWidget::mouseMoveEvent( event );
    update();
}

void Viewer::ImageDisplay::mouseReleaseEvent( QMouseEvent* event )
{
    // enable cursor hiding and reset timer
    enableCursorHiding();
    showCursor();

    _cache.remove( _curIndex );
    QMouseEvent e( event->type(), mapPos( event->pos() ), event->button(), event->buttons(), event->modifiers() );
    double ratio = sizeRatio( QSize(_zEnd.x()-_zStart.x(), _zEnd.y()-_zStart.y()), size() );
    bool block = _viewHandler->mouseReleaseEvent( &e, event->pos(), ratio );
    if ( !block ) {
        QWidget::mouseReleaseEvent( event );
    }
    emit possibleChange();
    update();
}

bool Viewer::ImageDisplay::setImage( DB::ImageInfoPtr info, bool forward )
{
    _info = info;
    _loadedImage = QImage();

    // Find the index of the current image
    _curIndex = 0;
    for( DB::FileNameList::Iterator it = _imageList.begin(); it != _imageList.end(); ++it ) {
        if ( *it == info->fileName() )
            break;
        ++_curIndex;
    }

    ViewPreloadInfo* found = _cache[_curIndex];
    if ( found && found->angle == info->angle() ) {
        _loadedImage = found->img;
        updateZoomPoints( Settings::SettingsData::instance()->viewerStandardSize(), found->img.size() );
        cropAndScale();
        info->setSize( found->size );
        emit imageReady();
    }
    else {
        requestImage( info, true );
        busy();
    }
    _forward = forward;
    updatePreload();

    return true;
}

void Viewer::ImageDisplay::resizeEvent( QResizeEvent* event )
{
    ImageManager::AsyncLoader::instance()->stop( this, ImageManager::StopOnlyNonPriorityLoads );
    _cache.fill(0); // Clear the cache
    if ( _info ) {
        cropAndScale();
        if ( event->size().width() > 1.5*this->_loadedImage.size().width() || event->size().height() > 1.5*this->_loadedImage.size().height() )
            potentialyLoadFullSize(); // Only do if we scale much bigger.
    }
    updatePreload();
}

void Viewer::ImageDisplay::paintEvent( QPaintEvent* )
{
    int x = ( width() - _croppedAndScaledImg.width() ) / 2;
    int y = ( height() - _croppedAndScaledImg.height() ) / 2;

    QPainter painter( this );
    painter.fillRect( 0,0, width(), height(), Qt::black );
    painter.drawImage( x,y, _croppedAndScaledImg );
}

QPoint Viewer::ImageDisplay::offset( int logicalWidth, int logicalHeight, int physicalWidth, int physicalHeight, double* ratio )
{
    double rat = sizeRatio( QSize( logicalWidth, logicalHeight ), QSize( physicalWidth, physicalHeight ) );

    int ox = (int) (physicalWidth - logicalWidth*rat)/2;
    int oy = (int) (physicalHeight - logicalHeight*rat)/2;
    if ( ratio )
        *ratio = rat;
    return QPoint(ox,oy);
}



void Viewer::ImageDisplay::zoom( QPoint p1, QPoint p2 )
{
    _cache.remove( _curIndex );
    normalize( p1, p2 );

    double ratio;
    QPoint off = offset( (p2-p1).x(), (p2-p1).y(), width(), height(), &ratio );
    off = off / ratio;

    p1.setX( p1.x() - off.x() );
    p1.setY( p1.y() - off.y() );
    p2.setX( p2.x()+off.x() );
    p2.setY( p2.y()+off.y() );

    _zStart = p1;
    _zEnd = p2;
    potentialyLoadFullSize();
    cropAndScale();
}

QPoint Viewer::ImageDisplay::mapPos( QPoint p )
{
    QPoint off = offset( qAbs( _zEnd.x()-_zStart.x() ), qAbs( _zEnd.y()-_zStart.y() ), width(), height(), 0 );
    p -= off;
    int x = (int) (_zStart.x() + (_zEnd.x()-_zStart.x())*((double)p.x()/ (width()-2*off.x())));
    int y = (int) (_zStart.y() + (_zEnd.y()-_zStart.y())*((double)p.y()/ (height()-2*off.y())));

    return QPoint( x, y );

}

void Viewer::ImageDisplay::xformPainter( QPainter* p )
{
    QPoint off = offset( qAbs( _zEnd.x()-_zStart.x() ), qAbs( _zEnd.y()-_zStart.y() ), width(), height(), 0 );
    double s = (width()-2*off.x())/qAbs( (double)_zEnd.x()-_zStart.x());
    p->scale( s, s );
    p->translate( -_zStart.x(), -_zStart.y() );
}

void Viewer::ImageDisplay::zoomIn()
{
    QPoint size = (_zEnd-_zStart);
    QPoint p1 = _zStart + size*(0.2/2);
    QPoint p2 = _zEnd - size*(0.2/2);
    zoom(p1, p2);
}

void Viewer::ImageDisplay::zoomOut()
{
    QPoint size = (_zEnd-_zStart);

    //Bug 150971, Qt tries to render bigger and bigger images (10000x10000), hence running out of memory.
    if ( ( size.x() * size.y() > 25*1024*1024 ) )
        return;

    QPoint p1 = _zStart - size*(0.25/2);
    QPoint p2 = _zEnd + size*(0.25/2);
    zoom(p1,p2);
}

void Viewer::ImageDisplay::zoomFull()
{
    _zStart = QPoint(0,0);
    _zEnd = QPoint( _loadedImage.width(), _loadedImage.height() );
    zoom( QPoint(0,0), QPoint( _loadedImage.width(), _loadedImage.height() ) );
}


void Viewer::ImageDisplay::normalize( QPoint& p1, QPoint& p2 )
{
    int minx = qMin( p1.x(), p2.x() );
    int miny = qMin( p1.y(), p2.y() );
    int maxx = qMax( p1.x(), p2.x() );
    int maxy = qMax( p1.y(), p2.y() );
    p1 = QPoint( minx, miny );
    p2 = QPoint( maxx, maxy );
}

void Viewer::ImageDisplay::pan( const QPoint& point )
{
    _zStart += point;
    _zEnd += point;
    cropAndScale();
}

void Viewer::ImageDisplay::cropAndScale()
{
    if ( _loadedImage.isNull() ) {
        return;
    }

    if ( _zStart != QPoint(0,0) || _zEnd != QPoint( _loadedImage.width(), _loadedImage.height() ) ) {
        _croppedAndScaledImg = _loadedImage.copy( _zStart.x(), _zStart.y(), _zEnd.x() - _zStart.x(), _zEnd.y() - _zStart.y() );
    }
    else
        _croppedAndScaledImg = _loadedImage;

    updateZoomCaption();

    if ( !_croppedAndScaledImg.isNull() ) // I don't know how this can happen, but it seems not to be dangerous.
        _croppedAndScaledImg = _croppedAndScaledImg.scaled( width(), height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    update();
}

void Viewer::ImageDisplay::filterNone()
{
    cropAndScale();
    update();
}

bool Viewer::ImageDisplay::filterMono()
{
    _croppedAndScaledImg = _croppedAndScaledImg.convertToFormat(_croppedAndScaledImg.Format_Mono);
    update();
    return true;
}

// I can't believe there isn't a standard conversion for this??? -- WH
bool Viewer::ImageDisplay::filterBW()
{
    if (_croppedAndScaledImg.depth() < 32) {
        KMessageBox::error( this, i18n("Insufficient color depth for this filter"));
        return false;
    }

    for (int y = 0; y < _croppedAndScaledImg.height(); ++y) {
        for (int x = 0; x < _croppedAndScaledImg.width(); ++x) {
            int pixel = _croppedAndScaledImg.pixel(x, y);
            int gray = qGray(pixel);
            int alpha = qAlpha(pixel);
            _croppedAndScaledImg.setPixel(x, y, qRgba(gray, gray, gray, alpha));
        }
    }
    update();
    return true;
}

bool Viewer::ImageDisplay::filterContrastStretch()
{
    int redMin, redMax, greenMin, greenMax, blueMin, blueMax;

    redMin = greenMin = blueMin = 255;
    redMax = greenMax = blueMax = 0;

    if (_croppedAndScaledImg.depth() < 32) {
        KMessageBox::error( this, i18n("Insufficient color depth for this filter"));
        return false;
    }

    // Look for minimum and maximum intensities within each color channel
    for (int y = 0; y < _croppedAndScaledImg.height(); ++y) {
        for (int x = 0; x < _croppedAndScaledImg.width(); ++x) {
            int pixel = _croppedAndScaledImg.pixel(x, y);
            int red = qRed(pixel);
            int green = qGreen(pixel);
            int blue = qBlue(pixel);
            redMin = redMin < red ? redMin : red;
            redMax = redMax > red ? redMax : red;
            greenMin = greenMin < green ? greenMin : green;
            greenMax = greenMax > green ? greenMax : green;
            blueMin = blueMin < blue ? blueMin : blue;
            blueMax = blueMax > blue ? blueMax : blue;
        }
    }

    // Calculate factor for stretching each color intensity throughout the
    // whole range
    float redFactor, greenFactor, blueFactor;
    redFactor = ((float)(255) / (float) (redMax - redMin));
    greenFactor = ((float)(255) / (float) (greenMax - greenMin));
    blueFactor = ((float)(255) / (float) (blueMax - blueMin));

    // Perform the contrast stretching
    for (int y = 0; y < _croppedAndScaledImg.height(); ++y) {
        for (int x = 0; x < _croppedAndScaledImg.width(); ++x) {
            int pixel = _croppedAndScaledImg.pixel(x, y);
            int red = qRed(pixel);
            int green = qGreen(pixel);
            int blue = qBlue(pixel);
            int alpha = qAlpha(pixel);

            red = (red - redMin) * redFactor;
            red = red < 255 ? red : 255;
            red = red > 0 ? red : 0;
            green = (green - greenMin) * greenFactor;
            green = green < 255 ? green : 255;
            green = green > 0 ? green : 0;
            blue = (blue - blueMin) * blueFactor;
            blue = blue < 255 ? blue : 255;
            blue = blue > 0 ? blue : 0;
            _croppedAndScaledImg.setPixel(x, y, qRgba(red, green, blue, alpha));
        }
    }
    update();
    return true;
}

bool Viewer::ImageDisplay::filterHistogramEqualization()
{
    int width, height;
    float R_histogram[256];
    float G_histogram[256];
    float B_histogram[256];
    float d;

    if (_croppedAndScaledImg.depth() < 32) {
        KMessageBox::error( this, i18n("Insufficient color depth for this filter"));
        return false;
    }
    memset(R_histogram, 0, sizeof(R_histogram));
    memset(G_histogram, 0, sizeof(G_histogram));
    memset(B_histogram, 0, sizeof(B_histogram));

    width = _croppedAndScaledImg.width();
    height = _croppedAndScaledImg.height();
    d = 1.0 / width / height;

    // Populate histogram for each color channel
    for (int y = 0; y < height; ++y) {
        for (int x = 1; x < width; ++x) {
            int pixel = _croppedAndScaledImg.pixel(x, y);

            R_histogram[qRed(pixel)] += d;
            G_histogram[qGreen(pixel)] += d;
            B_histogram[qBlue(pixel)] += d;
        }
    }

    // Transfer histogram table to cumulative distribution table
    float R_sum = 0.0;
    float G_sum = 0.0;
    float B_sum = 0.0;
    for (int i = 0; i < 256; ++i) {
        R_sum += R_histogram[i];
        G_sum += G_histogram[i];
        B_sum += B_histogram[i];

        R_histogram[i] = R_sum * 255 + 0.5;
        G_histogram[i] = G_sum * 255 + 0.5;
        B_histogram[i] = B_sum * 255 + 0.5;

    }

    // Equalize the image
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int pixel = _croppedAndScaledImg.pixel(x, y);

            _croppedAndScaledImg.setPixel(
                x, y, qRgba(R_histogram[qRed(pixel)],
                G_histogram[qGreen(pixel)], B_histogram[qBlue(pixel)],
                qAlpha(pixel))
            );
        }
    }
    update();
    return true;
}

void Viewer::ImageDisplay::updateZoomCaption() {
    const QSize imgSize = _loadedImage.size();
    // similar to sizeRatio(), but we take the _highest_ factor.
    double ratio = ((double)imgSize.width())/(_zEnd.x()-_zStart.x());
    if ( ratio * (_zEnd.y()-_zStart.y()) < imgSize.height() ) {
        ratio = ((double)imgSize.height())/(_zEnd.y()-_zStart.y());
    }

    emit setCaptionInfo((ratio > 1.05)
                        ? ki18n("[ zoom x%1 ]").subs(ratio, 0, 'f', 1).toString()
                        : QString());
}

QImage Viewer::ImageDisplay::currentViewAsThumbnail() const
{
    if ( _croppedAndScaledImg.isNull() )
        return QImage();
    else
        return _croppedAndScaledImg.scaled( 512, 512, Qt::KeepAspectRatio, Qt::SmoothTransformation );
}


bool Viewer::ImageDisplay::isImageZoomed( const Settings::StandardViewSize type, const QSize& imgSize )
{
    if (type == Settings::FullSize)
        return true;

    if ( type == Settings::NaturalSizeIfFits )
        return !(imgSize.width() < width() && imgSize.height() < height() );

    return false;
}

void Viewer::ImageDisplay::pixmapLoaded( const DB::FileName& fileName, const QSize& imgSize, const QSize& fullSize, int angle,
                                         const QImage& img, const bool loadedOK)
{
    if ( loadedOK && fileName == _info->fileName() ) {
        if ( fullSize.isValid() && !_info->size().isValid() )
            _info->setSize( fullSize );

        if ( !_reloadImageInProgress )
            updateZoomPoints( Settings::SettingsData::instance()->viewerStandardSize(), img.size() );
        else {
            // See documentation for zoomPixelForPixel for details.
            // We just loaded a likel much larger image, so the zoom points
            // need to be scaled. Notice _loadedImage is the size of the
            // old image.
            double ratio = sizeRatio( _loadedImage.size(), _info->size() );

            _zStart *= ratio;
            _zEnd *= ratio;

            _reloadImageInProgress = false;
        }

        _loadedImage = img;
        cropAndScale();
        emit imageReady();
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

void Viewer::ImageDisplay::setImageList( const DB::FileNameList& list )
{
    _imageList = list;
    _cache.fill( 0, list.count() );
}

void Viewer::ImageDisplay::updatePreload()
{
    uint cacheSize = ( Settings::SettingsData::instance()->viewerCacheSize() * 1024 * 1024 ) / (width()*height()*4);
    bool cacheFull = (_cache.count() > cacheSize);

    int incr = ( _forward ? 1 : -1 );
    int nextOnesInCache = 0;
    // Iterate from the current image in the direction of the viewing
    for ( int i = _curIndex+incr; cacheSize ; i += incr ) {
        if ( _forward ? ( i >= (int) _imageList.count() ) : (i < 0) )
            break;

        DB::ImageInfoPtr info = DB::ImageDB::instance()->info(_imageList[i]);
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
            requestImage( info );

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


int Viewer::ImageDisplay::indexOf( const DB::FileName& fileName )
{
    int i = 0;
    for( DB::FileNameList::ConstIterator it = _imageList.constBegin(); it != _imageList.constEnd(); ++it ) {
        if ( *it == fileName )
            break;
        ++i;
    }
    return i;
}

void Viewer::ImageDisplay::busy()
{
    if ( !_busy )
        qApp->setOverrideCursor( Qt::WaitCursor );
    _busy = true;
}

void Viewer::ImageDisplay::unbusy()
{
    if ( _busy )
        qApp->restoreOverrideCursor();
    _busy = false;
}

void Viewer::ImageDisplay::zoomPixelForPixel()
{
    // This is rather tricky.
    // We want to zoom to a pixel level for the real image, which we might
    // or might not have loaded yet.
    //
    // First we ask for zoom points as they would look like had we had the
    // real image loaded now. (We need to ask for them, for the real image,
    // otherwise we would just zoom to the pixel level of the view size
    // image)
    updateZoomPoints( Settings::NaturalSize, _info->size() );

    // The points now, however might not match the current visible image -
    // as this image might be be only view size large. We therefore need
    // to scale the coordinates.
    double ratio = sizeRatio( _loadedImage.size(), _info->size() );
    _zStart /= ratio;
    _zEnd /= ratio;
    cropAndScale();
    potentialyLoadFullSize();
}

void Viewer::ImageDisplay::updateZoomPoints( const Settings::StandardViewSize type, const QSize& imgSize )
{
    const int iw = imgSize.width();
    const int ih = imgSize.height();

    if ( isImageZoomed( type,  imgSize ) ) {
        _zStart=QPoint( 0, 0 );
        _zEnd=QPoint(  iw, ih );
    }
    else {
        _zStart = QPoint( - ( width()-iw ) / 2, -(height()-ih)/2);
        _zEnd = QPoint( iw + (width()-iw)/2, ih+(height()-ih)/2);
    }
}

void Viewer::ImageDisplay::potentialyLoadFullSize()
{
    if ( _info->size() != _loadedImage.size() ) {
        ImageManager::ImageRequest* request = new ImageManager::ImageRequest( _info->fileName(), QSize(-1,-1), _info->angle(), this );
        request->setPriority( ImageManager::Viewer );
        ImageManager::AsyncLoader::instance()->load( request );
        busy();
        _reloadImageInProgress = true;
    }
}

/**
 * return the ratio of the two sizes. That is  newSize/baseSize.
 */
double Viewer::ImageDisplay::sizeRatio( const QSize& baseSize, const QSize& newSize ) const
{
    double res = ((double)newSize.width())/baseSize.width();

    if ( res * baseSize.height() > newSize.height() ) {
        res = ((double)newSize.height())/baseSize.height();
    }
    return res;
}

void Viewer::ImageDisplay::requestImage( const DB::ImageInfoPtr& info, bool priority )
{
    Settings::StandardViewSize viewSize = Settings::SettingsData::instance()->viewerStandardSize();
    QSize s = size();
    if ( viewSize == Settings::NaturalSize )
        s = QSize(-1,-1);

    ImageManager::ImageRequest* request = new ImageManager::ImageRequest( info->fileName(), s, info->angle(), this );
    request->setUpScale( viewSize == Settings::FullSize );
    request->setPriority( priority ? ImageManager::Viewer : ImageManager::ViewerPreload );
    ImageManager::AsyncLoader::instance()->load( request );
}

void Viewer::ImageDisplay::hideEvent(QHideEvent *)
{
  _viewHandler->hideEvent();
}

#include "ImageDisplay.moc"
