/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
#include "ImageRequest.h"
ImageManager::ImageRequest::ImageRequest() : _null( true ),  _cache( true ),  _client( 0 )
{
}

ImageManager::ImageRequest::ImageRequest( const QString& fileName, const QSize& size, int angle, ImageManager::ImageClient* client )
    : _null( false ),  _fileName( fileName ),  _width( size.width() ),  _height( size.height() ),
      _cache( false ),  _client( client ),  _angle( angle ), _priority( false ), _loadedOK( false ), _dontUpScale( false )
{
}

bool ImageManager::ImageRequest::loadedOK() const
{
    return _loadedOK;
}

bool ImageManager::ImageRequest::isNull() const
{
    return _null;
}

QString ImageManager::ImageRequest::fileName() const
{
    // We need a lock here to avoid a race condition in Operator T() of QDeepCopy
    QMutexLocker dummy( &_fileNameLock );
    return _fileName;
}

int ImageManager::ImageRequest::width() const
{
    return _width;
}

int ImageManager::ImageRequest::height() const
{
    return _height;
}

bool ImageManager::ImageRequest::operator<( const ImageRequest& other ) const
{
    if (  fileName() != other.fileName() )
        return fileName() < other.fileName();
    else if ( _width != other._width )
        return _width < other._width;
    else if ( _height < other._height )
        return _height < other._height;
    else
        return _angle < other._angle;
}

bool ImageManager::ImageRequest::operator==( const ImageRequest& other ) const
{
    // Compare all atributes but the pixmap.
    return ( _null == other._null && fileName() == other.fileName() &&
             _width == other._width && _height == other._height &&
             _angle == other._angle & _client == other._client &&
             _priority == other._priority );
}


void ImageManager::ImageRequest::setCache( bool b )
{
    _cache = b;
}

bool ImageManager::ImageRequest::cache() const
{
    return _cache;
}

ImageManager::ImageClient* ImageManager::ImageRequest::client() const
{
    return _client;
}

int ImageManager::ImageRequest::angle() const
{
    return _angle;
}

QSize ImageManager::ImageRequest::fullSize() const
{
    return _fullSize;
}

void ImageManager::ImageRequest::setFullSize( const QSize& size )
{
    _fullSize = size;
}

void ImageManager::ImageRequest::setLoadedOK( bool ok )
{
    _loadedOK = ok;
}

bool ImageManager::ImageRequest::priority() const
{
    return _priority;
}

void ImageManager::ImageRequest::setPriority( bool b )
{
    _priority = b;
}

bool ImageManager::ImageRequest::stillNeeded() const
{
    return true;
}

bool ImageManager::ImageRequest::doUpScale() const
{
    return !_dontUpScale;
}

void ImageManager::ImageRequest::setUpScale( bool b )
{
    _dontUpScale = !b;
}
