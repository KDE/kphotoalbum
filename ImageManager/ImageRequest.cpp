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
#include "ImageRequest.h"

ImageManager::ImageRequest::ImageRequest( const DB::FileName& fileName,
                                          const QSize& size, int angle,
                                          ImageManager::ImageClientInterface* client )
    : _null( false ),
      _fileName( fileName ),
      _width( size.width() ),
      _height( size.height() ),
      _client( client ),
      _angle( angle ),
      _priority( ThumbnailVisible ),
      _loadedOK( false ),
      _dontUpScale( false ),
      _isThumbnailRequest(false)
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
    const DB::FileName fileA = databaseFileName();
    const DB::FileName fileB = other.databaseFileName();

    if (  fileA != fileB )
        return fileA < fileB;
    else if ( _width != other._width )
        return _width < other._width;
    else if ( _height != other._height )
        return _height < other._height;
    else
        return _angle < other._angle;
}

bool ImageManager::ImageRequest::operator==( const ImageRequest& other ) const
{
    // Compare all atributes but the pixmap.
    return ( _null == other._null && databaseFileName() == other.databaseFileName() &&
             _width == other._width && _height == other._height &&
             _angle == other._angle && _client == other._client &&
             _priority == other._priority );
}

ImageManager::ImageClientInterface* ImageManager::ImageRequest::client() const
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

ImageManager::Priority ImageManager::ImageRequest::priority() const
{
    return _priority;
}

void ImageManager::ImageRequest::setPriority( const Priority prio )
{
    _priority = prio;
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

void ImageManager::ImageRequest::setIsThumbnailRequest( bool b )
{
    _isThumbnailRequest = b;
}

bool ImageManager::ImageRequest::isThumbnailRequest() const
{
    return _isThumbnailRequest;
}

DB::FileName ImageManager::ImageRequest::databaseFileName() const
{
    return _fileName;
}

DB::FileName ImageManager::ImageRequest::fileSystemFileName() const
{
    return _fileName;
}

QSize ImageManager::ImageRequest::size() const
{
    return QSize( _width, _height );
}

// vi:expandtab:tabstop=4 shiftwidth=4:
