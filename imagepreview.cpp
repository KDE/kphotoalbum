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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "imagepreview.h"
#include "viewer.h"
#include "imagemanager.h"
#include <klocale.h>
#include <qwmatrix.h>
#include "imageloader.h"

ImagePreview::ImagePreview( QWidget* parent, const char* name )
    : QLabel( parent, name )
{
    setAlignment( AlignCenter );
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
    if ( !_info.isNull() )
        _info.rotate( angle );
    else
        _angle += angle;
    _preloader.cancelPreload();
    _lastImage.reset();
    reload();
}

void ImagePreview::setImage( const ImageInfo& info )
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
    _info = ImageInfo();
    _angle = 0;
    reload();
}


void ImagePreview::reload()
{
    if ( !_info.isNull() ) {
        QImage img;
        if (_preloader.has(_info.fileName()))
            setCurrentImage(_preloader.getImage());
        else if (_lastImage.has(_info.fileName()))
            //don't pass by reference, the additional constructor is needed here
            //see setCurrentImage for the reason (where _lastImage is changed...)
            setCurrentImage(QImage(_lastImage.getImage()));
        else {
            setPixmap(QImage()); //erase old image
            ImageManager::instance()->stop(this);
            ImageRequest* request = new ImageRequest( _info.fileName(), QSize( width(), height() ), _info.angle(), this );
            request->setPriority();
            ImageManager::instance()->load( request );
        }
    }
    else {
        QImage img( _fileName );
        img = ImageLoader::rotateAndScale( img, width(), height(), _angle );
        setPixmap( img );
    }
}

int ImagePreview::angle() const
{
    Q_ASSERT( _info.isNull() );
    return _angle;
}

void ImagePreview::setCurrentImage(const QImage &image)
{
    //cache the current image as the last image before changing it
    _lastImage.set(_currentImage);
    _currentImage.set(_info.fileName(), image);
    setPixmap(_currentImage.getImage());
    if (!_anticipated._fileName.isNull())
        _preloader.preloadImage(_anticipated._fileName, width(), height(), _anticipated._angle);
}

void ImagePreview::pixmapLoaded( const QString& fileName, const QSize& /*size*/, const QSize& /*fullSize*/, int, const QImage& image, bool loadedOK)
{
    if ( loadedOK && !_info.isNull() ) {
        if (_info.fileName() == fileName)
            setCurrentImage(image);
    }
}

void ImagePreview::anticipate(ImageInfo &info1) {
    //We cannot call _preloader.preloadImage right here:
    //this function is called before reload(), so if we preload here,
    //the preloader will always be loading the image after the next image.
    _anticipated.set(info1.fileName(), info1.angle());
}


ImagePreview::PreloadInfo::PreloadInfo() : _angle(0)
{
}

void ImagePreview::PreloadInfo::set(const QString& fileName, int angle)
{
    _fileName=fileName;
    _angle=angle;
}


bool ImagePreview::PreviewImage::has(const QString &fileName) const
{
    return fileName==_fileName && !_image.isNull();
}

QImage &ImagePreview::PreviewImage::getImage()
{
    return _image;
}

const QString &ImagePreview::PreviewImage::getName() const
{
    return _fileName;
}

void ImagePreview::PreviewImage::set(const QString &fileName, const QImage &image)
{
    _fileName=fileName;
    _image=image;
}

void ImagePreview::PreviewImage::set(const PreviewImage &other)
{
    _fileName=other._fileName;
    _image=other._image;
}

void ImagePreview::PreviewImage::reset()
{
    _fileName=QString::null;
    _image=QImage();
}


void ImagePreview::PreviewLoader::pixmapLoaded( const QString& fileName, const QSize& /*size*/,
                                                const QSize& /*fullSize*/, int, const QImage& image, bool loadedOK )
{
    if ( loadedOK )
        set(fileName, image);
}


void ImagePreview::PreviewLoader::preloadImage(const QString &fileName, int width, int height, int angle)
{
    //no need to worry about concurrent access: everything happens in the event loop thread
    reset();
    ImageManager::instance()->stop(this);
    ImageRequest* request = new ImageRequest( fileName, QSize( width, height ), angle, this );
    request->setPriority();
    ImageManager::instance()->load( request );
}

void ImagePreview::PreviewLoader::cancelPreload()
{
    reset();
    ImageManager::instance()->stop(this);
}


#include "imagepreview.moc"
