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

#include <QRubberBand>
#include "ImagePreview.h"
#include "ImageManager/AsyncLoader.h"
#include "Utilities/Util.h"
#include "ResizableFrame.h"
#include <QImageReader>
#include <QDebug>

using namespace AnnotationDialog;

ImagePreview::ImagePreview( QWidget* parent )
    : QLabel( parent ), _selectionRect(0), _areaCreationEnabled( false )
{
    setAlignment( Qt::AlignCenter );
    setMinimumSize( 64, 64 );
}

void ImagePreview::resizeEvent( QResizeEvent* )
{
    _preloader.cancelPreload();
    _lastImage.reset();
    reload();
}

QSize ImagePreview::sizeHint() const
{
    return QSize( 128,128 );
}

void ImagePreview::rotate(int angle)
{
    if (! _info.isNull()) {
        _currentImage.setAngle( _info.angle() );
        _info.rotate( angle, DB::RotateImageInfoOnly );
    } else {
        // Can this really happen?
         _angle += angle;
    }

     _preloader.cancelPreload();
     _lastImage.reset();
     reload();

    rotateAreas(angle);
}

void ImagePreview::setImage( const DB::ImageInfo& info )
{
    _info = info;
    reload();
}

/**
   This method should only be used for the non-user images. Currently this includes
   two images: the search image and the configure several images at a time image.
*/
void ImagePreview::setImage( const QString& fileName )
{
    _fileName = fileName;
    _info = DB::ImageInfo();
    _angle = 0;
    // Set the current angle that will be passed to _lastImage
    _currentImage.setAngle( _info.angle() );
    reload();
}

void ImagePreview::reload()
{
    if ( !_info.isNull() ) {
        if (_preloader.has(_info.fileName(), _info.angle()))
            setCurrentImage(_preloader.getImage());
        else if (_lastImage.has(_info.fileName(), _info.angle()))
            //don't pass by reference, the additional constructor is needed here
            //see setCurrentImage for the reason (where _lastImage is changed...)
            setCurrentImage(QImage(_lastImage.getImage()));
        else {
            setPixmap(QPixmap()); //erase old image
            ImageManager::AsyncLoader::instance()->stop(this);
            ImageManager::ImageRequest* request = new ImageManager::ImageRequest( _info.fileName(), QSize( width(), height() ), _info.angle(), this );
            request->setPriority( ImageManager::Viewer );
            ImageManager::AsyncLoader::instance()->load( request );
        }
    }
    else {
        QImage img( _fileName );
        img = rotateAndScale( img, width(), height(), _angle );
        setPixmap( QPixmap::fromImage(img) );
    }
}

int ImagePreview::angle() const
{
    Q_ASSERT( !_info.isNull() );
    return _angle;
}

QSize ImagePreview::getActualImageSize()
{
    if (! _info.size().isValid()) {
        // We have to fetch the size from the image
        _info.setSize(QImageReader(_info.fileName().absolute()).size());
    }
    return _info.size();
}

void ImagePreview::setCurrentImage(const QImage &image)
{
    // Cache the current image as the last image before changing it
    _lastImage.set(_currentImage);

    _currentImage.set(_info.fileName(), image, _info.angle());
    setPixmap(QPixmap::fromImage(image));

     if (!_anticipated._fileName.isNull())
         _preloader.preloadImage(_anticipated._fileName, width(), height(), _anticipated._angle);

    // Calculate a scale factor from the original image's size and it's current preview
    QSize actualSize = getActualImageSize();
    QSize previewSize = _currentImage.getImage().size();
    _scaleWidth = double(actualSize.width()) / double(previewSize.width());
    _scaleHeight = double(actualSize.height()) / double(previewSize.height());

    // Calculate the min and max coordinates inside the preview widget
    int previewWidth = _currentImage.getImage().size().width();
    int previewHeight = _currentImage.getImage().size().height();
    int widgetWidth = this->frameGeometry().width();
    int widgetHeight = this->frameGeometry().height();
    _minX = (widgetWidth - previewWidth) / 2;
    _maxX = _minX + previewWidth - 1;
    _minY = (widgetHeight - previewHeight) / 2;
    _maxY = _minY + previewHeight - 1;

    // Put all areas to their respective position on the preview
    remapAreas();
}

void ImagePreview::pixmapLoaded(ImageManager::ImageRequest* request, const QImage& image)
{
    const DB::FileName fileName = request->databaseFileName();
    const bool loadedOK = request->loadedOK();

    if ( loadedOK && !_info.isNull() ) {
        if (_info.fileName() == fileName)
            setCurrentImage(image);
    }
}

void ImagePreview::anticipate(DB::ImageInfo &info1) {
    //We cannot call _preloader.preloadImage right here:
    //this function is called before reload(), so if we preload here,
    //the preloader will always be loading the image after the next image.
    _anticipated.set(info1.fileName(), info1.angle());
}


ImagePreview::PreloadInfo::PreloadInfo() : _angle(0)
{
}

void ImagePreview::PreloadInfo::set(const DB::FileName& fileName, int angle)
{
    _fileName=fileName;
    _angle=angle;
}


bool ImagePreview::PreviewImage::has(const DB::FileName &fileName, int angle) const
 {
    return fileName==_fileName && !_image.isNull() && angle==_angle;
}

QImage &ImagePreview::PreviewImage::getImage()
{
    return _image;
}

void ImagePreview::PreviewImage::set(const DB::FileName &fileName, const QImage &image, int angle)
 {
    _fileName = fileName;
    _image = image;
    _angle = angle;
}

void ImagePreview::PreviewImage::set(const PreviewImage &other)
{
    _fileName = other._fileName;
    _image = other._image;
    _angle = other._angle;
}

void ImagePreview::PreviewImage::setAngle( int angle )
{
    _angle = angle;
}

void ImagePreview::PreviewImage::reset()
{
    _fileName = DB::FileName();
    _image=QImage();
}

void ImagePreview::PreviewLoader::pixmapLoaded(ImageManager::ImageRequest* request, const QImage& image)
{
    if ( request->loadedOK() )
    {
        const DB::FileName fileName = request->databaseFileName();
        set( fileName, image, request->angle() );
    }
}

void ImagePreview::PreviewLoader::preloadImage(const DB::FileName &fileName, int width, int height, int angle)
{
    //no need to worry about concurrent access: everything happens in the event loop thread
    reset();
    ImageManager::AsyncLoader::instance()->stop(this);
    ImageManager::ImageRequest* request = new ImageManager::ImageRequest( fileName, QSize( width, height ), angle, this );
    request->setPriority( ImageManager::ViewerPreload );
    ImageManager::AsyncLoader::instance()->load( request );
}

void ImagePreview::PreviewLoader::cancelPreload()
{
    reset();
    ImageManager::AsyncLoader::instance()->stop(this);
}

QImage AnnotationDialog::ImagePreview::rotateAndScale(QImage img, int width, int height, int angle) const
{
    if ( angle != 0 )  {
        QMatrix matrix;
        matrix.rotate( angle );
        img = img.transformed( matrix );
    }
    img = Utilities::scaleImage(img, width, height, Qt::KeepAspectRatio );
    return img;
}

void ImagePreview::mousePressEvent(QMouseEvent *event)
{
    if (! _areaCreationEnabled) {
        return;
    }

    if (event->button() & Qt::LeftButton) {
        if (! _selectionRect) {
            _selectionRect = new QRubberBand(QRubberBand::Rectangle, this);
        }

        _areaStart = event->pos();
        if (_areaStart.x() < _minX or _areaStart.x() > _maxX or
            _areaStart.y() < _minY or _areaStart.y() > _maxY) {
            // Dragging started outside of the preview image
            return;
        }

        _selectionRect->setGeometry(QRect(_areaStart, QSize()));
        _selectionRect->show();
    }
}

void ImagePreview::mouseMoveEvent(QMouseEvent *event)
{
    if (! _areaCreationEnabled) {
        return;
    }

    if (_selectionRect && _selectionRect->isVisible()) {
        _currentPos = event->pos();

        // Restrict the coordinates to the preview images's size
        if (_currentPos.x() < _minX) {
            _currentPos.setX(_minX);
        }
        if (_currentPos.y() < _minY) {
            _currentPos.setY(_minY);
        }
        if (_currentPos.x() > _maxX) {
            _currentPos.setX(_maxX);
        }
        if (_currentPos.y() > _maxY) {
            _currentPos.setY(_maxY);
        }

        _selectionRect->setGeometry(QRect( _areaStart, _currentPos ).normalized());
    }
}

void ImagePreview::mouseReleaseEvent(QMouseEvent *event)
{
    if (! _areaCreationEnabled) {
        return;
    }

    if (event->button() & Qt::LeftButton and _selectionRect->isVisible()) {
        _areaEnd = event->pos();
        processNewArea();
        _selectionRect->hide();
    }
}

QRect ImagePreview::areaPreviewToActual(QRect area) const
{
    return QRect(QPoint(int(double(area.left() - _minX) * _scaleWidth),
                        int(double(area.top() - _minY) * _scaleHeight)),
                 QPoint(int(double(area.right() - _minX) * _scaleWidth),
                        int(double(area.bottom() - _minY) * _scaleHeight)));
}

QRect ImagePreview::areaActualToPreview(QRect area) const
{
    return QRect(QPoint(int(double(area.left() / _scaleWidth)) + _minX,
                        int(double(area.top() / _scaleHeight)) + _minY),
                 QPoint(int(double(area.right() / _scaleWidth)) + _minX,
                        int(double(area.bottom() / _scaleHeight)) + _minY));
}

void ImagePreview::createNewArea(QRect geometry, QRect actualGeometry)
{
    // Create a ResizableFrame (cleaned up in Dialog::tidyAreas())
    ResizableFrame *newArea = new ResizableFrame(this);
    emit areaCreated(newArea);

    newArea->setGeometry(geometry);
    // Be sure not to create an invisible area
    newArea->checkGeometry();
    // In case the geometry has been changed by checkGeometry()
    actualGeometry = areaPreviewToActual(newArea->geometry());
    // Store the coordinates on the real image (not on the preview)
    newArea->setActualCoordinates(actualGeometry);

    newArea->show();
    newArea->checkShowContextMenu();
}

void ImagePreview::processNewArea()
{
    if (_areaStart == _areaEnd) {
        // It was just a click, no area has been dragged
        return;
    }

    QRect newAreaPreview = QRect(_areaStart, _currentPos).normalized();
    createNewArea(newAreaPreview, areaPreviewToActual(newAreaPreview));
}

void ImagePreview::remapAreas()
{
    QList<ResizableFrame *> allAreas = this->findChildren<ResizableFrame *>();

    if (allAreas.isEmpty()) {
        return;
    }

    foreach (ResizableFrame *area, allAreas) {
        area->setGeometry(areaActualToPreview(area->actualCoordinates()));
    }
}

QRect ImagePreview::rotateArea(QRect originalAreaGeometry, int angle)
{
    // This is the current state of the image. We need the state before, so ...
    QSize unrotatedOriginalImageSize = getActualImageSize();
    // ... un-rotate it
    unrotatedOriginalImageSize.transpose();

    QRect rotatedAreaGeometry;
    rotatedAreaGeometry.setWidth(originalAreaGeometry.height());
    rotatedAreaGeometry.setHeight(originalAreaGeometry.width());

    if (angle == 90) {
        rotatedAreaGeometry.moveTo(
            unrotatedOriginalImageSize.height() - (originalAreaGeometry.height() + originalAreaGeometry.y()),
            originalAreaGeometry.x()
        );
    } else {
        rotatedAreaGeometry.moveTo(
            originalAreaGeometry.y(),
            unrotatedOriginalImageSize.width() - (originalAreaGeometry.width() + originalAreaGeometry.x())
        );
    }

    return rotatedAreaGeometry;
}

void ImagePreview::rotateAreas(int angle)
{
    // Map all areas to their respective coordinates on the rotated actual image
    QList<ResizableFrame *> allAreas = this->findChildren<ResizableFrame *>();
    foreach (ResizableFrame *area, allAreas) {
        area->setActualCoordinates(rotateArea(area->actualCoordinates(), angle));
    }
}

QRect ImagePreview::minMaxAreaPreview() const
{
    return QRect(_minX, _minY, _maxX, _maxY);
}

void ImagePreview::createTaggedArea(QString category, QString tag, QRect geometry, bool showArea)
{
    // Create a ResizableFrame (cleaned up in Dialog::tidyAreas())
    ResizableFrame *newArea = new ResizableFrame(this);

    emit areaCreated(newArea);

    newArea->setGeometry(areaActualToPreview(geometry));
    newArea->setActualCoordinates(geometry);
    newArea->setTagData(category, tag);
    newArea->setVisible(showArea);
}

void ImagePreview::setAreaCreationEnabled(bool state)
{
    _areaCreationEnabled = state;
}

#include "ImagePreview.moc"
// vi:expandtab:tabstop=4 shiftwidth=4:
