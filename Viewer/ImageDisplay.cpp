/* Copyright (C) 2003-2018 Jesper K. Pedersen <blackie@kde.org>

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
#include "Logging.h"

#include <QApplication>
#include <QCursor>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QTimer>

#include <KLocalizedString>
#include <KMessageBox>

#include "DB/ImageDB.h"
#include "ImageManager/AsyncLoader.h"
#include "Settings/SettingsData.h"
#include "Viewer/ViewHandler.h"

#include <cmath>

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

   The above might very likely be simplified. Back in the old days it needed to be that
   complex to allow drawing on images.

   To propagate the cache, we need to know which direction the
   images are viewed in, which is the job of the instance variable _forward.
*/

Viewer::ImageDisplay::ImageDisplay( QWidget* parent)
    :AbstractDisplay( parent ), m_reloadImageInProgress( false ), m_forward(true), m_curIndex(0),m_busy( false ),
    m_cursorHiding(true)
{
    m_viewHandler = new ViewHandler( this );

    setMouseTracking( true );
    m_cursorTimer = new QTimer( this );
    m_cursorTimer->setSingleShot(true);
    connect(m_cursorTimer, &QTimer::timeout, this, &ImageDisplay::hideCursor);
    showCursor();
}

/**
 * If mouse cursor hiding is enabled, hide the cursor right now
 */
void Viewer::ImageDisplay::hideCursor() {
    if (m_cursorHiding)
        setCursor( Qt::BlankCursor );
}

/**
 * If mouse cursor hiding is enabled, show normal cursor and start a timer that will hide it later
 */
void Viewer::ImageDisplay::showCursor() {
    if (m_cursorHiding) {
        unsetCursor();
        m_cursorTimer->start( 1500 );
    }
}

/**
 * Prevent hideCursor() and showCursor() from altering cursor state
 */
void Viewer::ImageDisplay::disableCursorHiding() {
    m_cursorHiding = false;
}

/**
 * Enable automatic mouse cursor hiding
 */
void Viewer::ImageDisplay::enableCursorHiding() {
    m_cursorHiding = true;
}

void Viewer::ImageDisplay::mousePressEvent( QMouseEvent* event )
{
    // disable cursor hiding till button release
    disableCursorHiding();

    QMouseEvent e( event->type(), mapPos( event->pos() ), event->button(), event->buttons(), event->modifiers() );
    double ratio = sizeRatio( QSize(m_zEnd.x()-m_zStart.x(), m_zEnd.y()-m_zStart.y()), size() );
    bool block = m_viewHandler->mousePressEvent( &e, event->pos(), ratio );
    if ( !block )
        QWidget::mousePressEvent( event );
    update();
}

void Viewer::ImageDisplay::mouseMoveEvent( QMouseEvent* event )
{
    // just reset the timer
    showCursor();

    QMouseEvent e( event->type(), mapPos( event->pos() ), event->button(), event->buttons(), event->modifiers() );
    double ratio = sizeRatio( QSize(m_zEnd.x()-m_zStart.x(), m_zEnd.y()-m_zStart.y()), size() );
    bool block = m_viewHandler->mouseMoveEvent( &e, event->pos(), ratio );
    if ( !block )
        QWidget::mouseMoveEvent( event );
    update();
}

void Viewer::ImageDisplay::mouseReleaseEvent( QMouseEvent* event )
{
    // enable cursor hiding and reset timer
    enableCursorHiding();
    showCursor();

    m_cache.remove( m_curIndex );
    QMouseEvent e( event->type(), mapPos( event->pos() ), event->button(), event->buttons(), event->modifiers() );
    double ratio = sizeRatio( QSize(m_zEnd.x()-m_zStart.x(), m_zEnd.y()-m_zStart.y()), size() );
    bool block = m_viewHandler->mouseReleaseEvent( &e, event->pos(), ratio );
    if ( !block ) {
        QWidget::mouseReleaseEvent( event );
    }
    emit possibleChange();
    update();
}

bool Viewer::ImageDisplay::setImage( DB::ImageInfoPtr info, bool forward )
{
    qCDebug(ViewerLog) << "setImage(" << info->fileName().relative() << "," << forward <<")";
    m_info = info;
    m_loadedImage = QImage();

    // Find the index of the current image
    m_curIndex = 0;
    Q_FOREACH( const DB::FileName &filename, m_imageList) {
        if ( filename == info->fileName() )
            break;
        ++m_curIndex;
    }

    if ( m_cache.contains(m_curIndex) && m_cache[m_curIndex].angle == info->angle()) {
        const ViewPreloadInfo& found = m_cache[m_curIndex];
        m_loadedImage = found.img;
        updateZoomPoints( Settings::SettingsData::instance()->viewerStandardSize(), found.img.size() );
        cropAndScale();
        info->setSize( found.size );
        emit imageReady();
    }
    else {
        requestImage( info, true );
        busy();
    }
    m_forward = forward;
    updatePreload();

    return true;
}

void Viewer::ImageDisplay::resizeEvent( QResizeEvent* event )
{
    ImageManager::AsyncLoader::instance()->stop( this, ImageManager::StopOnlyNonPriorityLoads );
    m_cache.clear();
    if ( m_info ) {
        cropAndScale();
        if ( event->size().width() > 1.5*this->m_loadedImage.size().width() || event->size().height() > 1.5*this->m_loadedImage.size().height() )
            potentiallyLoadFullSize(); // Only do if we scale much bigger.
    }
    updatePreload();
}

void Viewer::ImageDisplay::paintEvent( QPaintEvent* )
{
    int x = ( width() - m_croppedAndScaledImg.width() ) / 2;
    int y = ( height() - m_croppedAndScaledImg.height() ) / 2;

    QPainter painter( this );
    painter.fillRect( 0,0, width(), height(), Qt::black );
    painter.drawImage( x,y, m_croppedAndScaledImg );
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
    qCDebug(ViewerLog, "zoom(%d,%d, %d,%d)",p1.x(),p1.y(),p2.x(),p2.y());
    m_cache.remove( m_curIndex );
    normalize( p1, p2 );

    double ratio;
    QPoint off = offset( (p2-p1).x(), (p2-p1).y(), width(), height(), &ratio );
    off = off / ratio;

    p1.setX( p1.x() - off.x() );
    p1.setY( p1.y() - off.y() );
    p2.setX( p2.x()+off.x() );
    p2.setY( p2.y()+off.y() );

    m_zStart = p1;
    m_zEnd = p2;
    potentiallyLoadFullSize();
    cropAndScale();
}

QPoint Viewer::ImageDisplay::mapPos( QPoint p )
{
    QPoint off = offset( qAbs( m_zEnd.x()-m_zStart.x() ), qAbs( m_zEnd.y()-m_zStart.y() ), width(), height(), 0 );
    p -= off;
    int x = (int) (m_zStart.x() + (m_zEnd.x()-m_zStart.x())*((double)p.x()/ (width()-2*off.x())));
    int y = (int) (m_zStart.y() + (m_zEnd.y()-m_zStart.y())*((double)p.y()/ (height()-2*off.y())));

    return QPoint( x, y );

}

void Viewer::ImageDisplay::xformPainter( QPainter* p )
{
    QPoint off = offset( qAbs( m_zEnd.x()-m_zStart.x() ), qAbs( m_zEnd.y()-m_zStart.y() ), width(), height(), 0 );
    double s = (width()-2*off.x())/qAbs( (double)m_zEnd.x()-m_zStart.x());
    p->scale( s, s );
    p->translate( -m_zStart.x(), -m_zStart.y() );
}

void Viewer::ImageDisplay::zoomIn()
{
    qCDebug(ViewerLog, "zoomIn()");
    QPoint size = (m_zEnd-m_zStart);
    QPoint p1 = m_zStart + size*(0.2/2);
    QPoint p2 = m_zEnd - size*(0.2/2);
    zoom(p1, p2);
}

void Viewer::ImageDisplay::zoomOut()
{
    qCDebug(ViewerLog, "zoomOut()");
    QPoint size = (m_zEnd-m_zStart);

    //Bug 150971, Qt tries to render bigger and bigger images (10000x10000), hence running out of memory.
    if ( ( size.x() * size.y() > 25*1024*1024 ) )
        return;

    QPoint p1 = m_zStart - size*(0.25/2);
    QPoint p2 = m_zEnd + size*(0.25/2);
    zoom(p1,p2);
}

void Viewer::ImageDisplay::zoomFull()
{
    qCDebug(ViewerLog, "zoomFull()");
    m_zStart = QPoint(0,0);
    m_zEnd = QPoint( m_loadedImage.width(), m_loadedImage.height() );
    zoom( QPoint(0,0), QPoint( m_loadedImage.width(), m_loadedImage.height() ) );
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
    m_zStart += point;
    m_zEnd += point;
    cropAndScale();
}

void Viewer::ImageDisplay::cropAndScale()
{
    if ( m_loadedImage.isNull() ) {
        return;
    }

    if ( m_zStart != QPoint(0,0) || m_zEnd != QPoint( m_loadedImage.width(), m_loadedImage.height() ) ) {
        qCDebug(ViewerLog) << "cropAndScale(): using cropped image" << m_zStart << "-" << m_zEnd;
        m_croppedAndScaledImg = m_loadedImage.copy( m_zStart.x(), m_zStart.y(), m_zEnd.x() - m_zStart.x(), m_zEnd.y() - m_zStart.y() );
    }
    else
    {
        qCDebug(ViewerLog) << "cropAndScale(): using full image.";
        m_croppedAndScaledImg = m_loadedImage;
    }

    updateZoomCaption();

    if ( !m_croppedAndScaledImg.isNull() ) // I don't know how this can happen, but it seems not to be dangerous.
    {
        qCDebug(ViewerLog) << "cropAndScale(): scaling image to" << width() <<"x"<< height();
        m_croppedAndScaledImg = m_croppedAndScaledImg.scaled( width(), height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    } else {
        qCDebug(ViewerLog) << "cropAndScale(): image is null.";
    }

    update();

    emit viewGeometryChanged(m_croppedAndScaledImg.size(), QRect(m_zStart, m_zEnd), sizeRatio(m_loadedImage.size(), m_info->size()));
}

void Viewer::ImageDisplay::filterNone()
{
    cropAndScale();
    update();
}

bool Viewer::ImageDisplay::filterMono()
{
    m_croppedAndScaledImg = m_croppedAndScaledImg.convertToFormat(m_croppedAndScaledImg.Format_Mono);
    update();
    return true;
}

// I can't believe there isn't a standard conversion for this??? -- WH
bool Viewer::ImageDisplay::filterBW()
{
    if (m_croppedAndScaledImg.depth() < 32) {
        KMessageBox::error( this, i18n("Insufficient color depth for this filter"));
        return false;
    }

    for (int y = 0; y < m_croppedAndScaledImg.height(); ++y) {
        for (int x = 0; x < m_croppedAndScaledImg.width(); ++x) {
            int pixel = m_croppedAndScaledImg.pixel(x, y);
            int gray = qGray(pixel);
            int alpha = qAlpha(pixel);
            m_croppedAndScaledImg.setPixel(x, y, qRgba(gray, gray, gray, alpha));
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

    if (m_croppedAndScaledImg.depth() < 32) {
        KMessageBox::error( this, i18n("Insufficient color depth for this filter"));
        return false;
    }

    // Look for minimum and maximum intensities within each color channel
    for (int y = 0; y < m_croppedAndScaledImg.height(); ++y) {
        for (int x = 0; x < m_croppedAndScaledImg.width(); ++x) {
            int pixel = m_croppedAndScaledImg.pixel(x, y);
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
    for (int y = 0; y < m_croppedAndScaledImg.height(); ++y) {
        for (int x = 0; x < m_croppedAndScaledImg.width(); ++x) {
            int pixel = m_croppedAndScaledImg.pixel(x, y);
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
            m_croppedAndScaledImg.setPixel(x, y, qRgba(red, green, blue, alpha));
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

    if (m_croppedAndScaledImg.depth() < 32) {
        KMessageBox::error( this, i18n("Insufficient color depth for this filter"));
        return false;
    }
    memset(R_histogram, 0, sizeof(R_histogram));
    memset(G_histogram, 0, sizeof(G_histogram));
    memset(B_histogram, 0, sizeof(B_histogram));

    width = m_croppedAndScaledImg.width();
    height = m_croppedAndScaledImg.height();
    d = 1.0 / width / height;

    // Populate histogram for each color channel
    for (int y = 0; y < height; ++y) {
        for (int x = 1; x < width; ++x) {
            int pixel = m_croppedAndScaledImg.pixel(x, y);

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
            int pixel = m_croppedAndScaledImg.pixel(x, y);

            m_croppedAndScaledImg.setPixel(
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
    const QSize imgSize = m_loadedImage.size();
    // similar to sizeRatio(), but we take the _highest_ factor.
    double ratio = ((double)imgSize.width())/(m_zEnd.x()-m_zStart.x());
    if ( ratio * (m_zEnd.y()-m_zStart.y()) < imgSize.height() ) {
        ratio = ((double)imgSize.height())/(m_zEnd.y()-m_zStart.y());
    }

    emit setCaptionInfo((ratio > 1.05)
                        ? ki18n("[ zoom x%1 ]").subs(ratio, 0, 'f', 1).toString()
                        : QString());
}

QImage Viewer::ImageDisplay::currentViewAsThumbnail() const
{
    if ( m_croppedAndScaledImg.isNull() )
        return QImage();
    else
        return m_croppedAndScaledImg.scaled( 512, 512, Qt::KeepAspectRatio, Qt::SmoothTransformation );
}


bool Viewer::ImageDisplay::isImageZoomed( const Settings::StandardViewSize type, const QSize& imgSize )
{
    if (type == Settings::FullSize)
        return true;

    if ( type == Settings::NaturalSizeIfFits )
        return !(imgSize.width() < width() && imgSize.height() < height() );

    return false;
}

void Viewer::ImageDisplay::pixmapLoaded(ImageManager::ImageRequest* request, const QImage& image)
{
    const DB::FileName fileName = request->databaseFileName();
    const QSize imgSize = request->size();
    const QSize fullSize = request->fullSize();
    const int angle = request->angle();
    const bool loadedOK = request->loadedOK();

    if ( loadedOK && fileName == m_info->fileName() ) {
        if ( fullSize.isValid() && !m_info->size().isValid() )
            m_info->setSize( fullSize );

        if ( !m_reloadImageInProgress )
            updateZoomPoints( Settings::SettingsData::instance()->viewerStandardSize(), image.size() );
        else {
            // See documentation for zoomPixelForPixel for details.
            // We just loaded a likely much larger image, so the zoom points
            // need to be scaled. Notice m_loadedImage is the size of the
            // old image.
            // when using raw images, the decoded image may be a preview
            // and have a size different from m_info->size(). Therefore, use fullSize here:
            double ratio = sizeRatio( m_loadedImage.size(), fullSize );

            qCDebug(ViewerLog) << "Old size:" << m_loadedImage.size() << "; new size:" << m_info->size();
            qCDebug(ViewerLog) << "Req size:" << imgSize << "fullsize:" << fullSize;
            qCDebug(ViewerLog) << "pixmapLoaded(): Zoom region was" << m_zStart <<"-"<<m_zEnd;
            m_zStart *= ratio;
            m_zEnd *= ratio;
            qCDebug(ViewerLog) << "pixmapLoaded(): Zoom region changed to" << m_zStart <<"-"<<m_zEnd;

            m_reloadImageInProgress = false;
        }

        m_loadedImage = image;
        cropAndScale();
        emit imageReady();
    }
    else {
        if ( imgSize != size() )
            return; // Might be an old preload version, or a loaded version that never made it in time

        ViewPreloadInfo info( image, fullSize, angle );
        m_cache.insert( indexOf(fileName), info );
        updatePreload();
    }
    unbusy();
    emit possibleChange();
}

void Viewer::ImageDisplay::setImageList( const DB::FileNameList& list )
{
    m_imageList = list;
    m_cache.clear();
}

void Viewer::ImageDisplay::updatePreload()
{
    // cacheSize: number of images at current window dimensions (at 4 byte per pixel)
    const int cacheSize = (int)
            ((long long) ( Settings::SettingsData::instance()->viewerCacheSize() * 1024LL * 1024LL ) / (width()*height()*4));
    bool cacheFull = (m_cache.count() > cacheSize);

    int incr = ( m_forward ? 1 : -1 );
    int nextOnesInCache = 0;
    // Iterate from the current image in the direction of the viewing
    for ( int i = m_curIndex+incr; cacheSize ; i += incr ) {
        if ( m_forward ? ( i >= (int) m_imageList.count() ) : (i < 0) )
            break;

        DB::ImageInfoPtr info = DB::ImageDB::instance()->info(m_imageList[i]);
        if ( !info ) {
            qCWarning(ViewerLog, "Info was null for index %d!", i);
            return;
        }

        if ( m_cache.contains(i) ) {
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
                for ( int j = ( m_forward ? 0 : m_imageList.count() -1 );
                      j != m_curIndex;
                      j += ( m_forward ? 1 : -1 ) ) {
                    if ( m_cache.contains(j) ) {
                        m_cache.remove(j);
                        return;
                    }
                }

                // OK We found no item in the direction we came from (think of home/end keys)
                for ( int j = ( m_forward ? m_imageList.count() -1 : 0 );
                      j != m_curIndex;
                      j += ( m_forward ? -1 : 1 ) ) {
                    if ( m_cache.contains(j) ) {
                        m_cache.remove(j);
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
    Q_FOREACH( const DB::FileName &name, m_imageList ) {
        if ( name == fileName )
            break;
        ++i;
    }
    return i;
}

void Viewer::ImageDisplay::busy()
{
    if ( !m_busy )
        qApp->setOverrideCursor( Qt::WaitCursor );
    m_busy = true;
}

void Viewer::ImageDisplay::unbusy()
{
    if ( m_busy )
        qApp->restoreOverrideCursor();
    m_busy = false;
}

void Viewer::ImageDisplay::zoomPixelForPixel()
{
    qCDebug(ViewerLog, "zoomPixelForPixel()");
    // This is rather tricky.
    // We want to zoom to a pixel level for the real image, which we might
    // or might not have loaded yet.
    //
    // First we ask for zoom points as they would look like had we had the
    // real image loaded now. (We need to ask for them, for the real image,
    // otherwise we would just zoom to the pixel level of the view size
    // image)
    updateZoomPoints( Settings::NaturalSize, m_info->size() );

    // The points now, however might not match the current visible image -
    // as this image might be be only view size large. We therefore need
    // to scale the coordinates.
    double ratio = sizeRatio( m_loadedImage.size(), m_info->size() );
    qCDebug(ViewerLog) << "zoomPixelForPixel(): Zoom region was" << m_zStart <<"-"<<m_zEnd;
    m_zStart /= ratio;
    m_zEnd /= ratio;
    qCDebug(ViewerLog) << "zoomPixelForPixel(): Zoom region changed to" << m_zStart <<"-"<<m_zEnd;
    cropAndScale();
    potentiallyLoadFullSize();
}

void Viewer::ImageDisplay::updateZoomPoints( const Settings::StandardViewSize type, const QSize& imgSize )
{
    const int iw = imgSize.width();
    const int ih = imgSize.height();

    if ( isImageZoomed( type,  imgSize ) ) {
        m_zStart=QPoint( 0, 0 );
        m_zEnd=QPoint(  iw, ih );
        qCDebug(ViewerLog) << "updateZoomPoints(): Zoom region reset to" << m_zStart <<"-"<<m_zEnd;
    }
    else {
        m_zStart = QPoint( - ( width()-iw ) / 2, -(height()-ih)/2);
        m_zEnd = QPoint( iw + (width()-iw)/2, ih+(height()-ih)/2);
        qCDebug(ViewerLog) << "updateZoomPoints(): Zoom region set to" << m_zStart <<"-"<<m_zEnd;
    }
}

void Viewer::ImageDisplay::potentiallyLoadFullSize()
{
    if ( m_info->size() != m_loadedImage.size() ) {
        qCDebug(ViewerLog) << "Loading full size image for " << m_info->fileName().relative();
        ImageManager::ImageRequest* request = new ImageManager::ImageRequest( m_info->fileName(), QSize(-1,-1), m_info->angle(), this );
        request->setPriority( ImageManager::Viewer );
        ImageManager::AsyncLoader::instance()->load( request );
        busy();
        m_reloadImageInProgress = true;
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
  m_viewHandler->hideEvent();
}

// vi:expandtab:tabstop=4 shiftwidth=4:
