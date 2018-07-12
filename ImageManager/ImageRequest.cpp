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
    : m_null( false ),
      m_fileName( fileName ),
      m_width( size.width() ),
      m_height( size.height() ),
      m_client( client ),
      m_angle( angle ),
      m_priority( ThumbnailVisible ),
      m_loadedOK( false ),
      m_dontUpScale( false ),
      m_isThumbnailRequest(false),
      m_isExitRequest(false)
{
}

ImageManager::ImageRequest::ImageRequest( bool requestExit )
    : m_isExitRequest( requestExit )
{
}

bool ImageManager::ImageRequest::isExitRequest() const
{
    return m_isExitRequest;
}

bool ImageManager::ImageRequest::loadedOK() const
{
    return m_loadedOK;
}

bool ImageManager::ImageRequest::isNull() const
{
    return m_null;
}

int ImageManager::ImageRequest::width() const
{
    return m_width;
}

int ImageManager::ImageRequest::height() const
{
    return m_height;
}

bool ImageManager::ImageRequest::operator<( const ImageRequest& other ) const
{
    const DB::FileName fileA = databaseFileName();
    const DB::FileName fileB = other.databaseFileName();

    if (  fileA != fileB )
        return fileA < fileB;
    else if ( m_width != other.m_width )
        return m_width < other.m_width;
    else if ( m_height != other.m_height )
        return m_height < other.m_height;
    else
        return m_angle < other.m_angle;
}

bool ImageManager::ImageRequest::operator==( const ImageRequest& other ) const
{
    // Compare all atributes but the pixmap.
    return ( m_null == other.m_null && databaseFileName() == other.databaseFileName() &&
             m_width == other.m_width && m_height == other.m_height &&
             m_angle == other.m_angle && m_client == other.m_client &&
             m_priority == other.m_priority );
}

ImageManager::ImageClientInterface* ImageManager::ImageRequest::client() const
{
    return m_client;
}

int ImageManager::ImageRequest::angle() const
{
    return m_angle;
}

QSize ImageManager::ImageRequest::fullSize() const
{
    return m_fullSize;
}

void ImageManager::ImageRequest::setFullSize( const QSize& size )
{
    m_fullSize = size;
}

void ImageManager::ImageRequest::setLoadedOK( bool ok )
{
    m_loadedOK = ok;
}

ImageManager::Priority ImageManager::ImageRequest::priority() const
{
    return m_priority;
}

void ImageManager::ImageRequest::setPriority( const Priority prio )
{
    m_priority = prio;
}

bool ImageManager::ImageRequest::stillNeeded() const
{
    return true;
}

bool ImageManager::ImageRequest::doUpScale() const
{
    return !m_dontUpScale;
}

void ImageManager::ImageRequest::setUpScale( bool b )
{
    m_dontUpScale = !b;
}

void ImageManager::ImageRequest::setIsThumbnailRequest( bool b )
{
    m_isThumbnailRequest = b;
}

bool ImageManager::ImageRequest::isThumbnailRequest() const
{
    return m_isThumbnailRequest;
}

DB::FileName ImageManager::ImageRequest::databaseFileName() const
{
    return m_fileName;
}

DB::FileName ImageManager::ImageRequest::fileSystemFileName() const
{
    return m_fileName;
}

QSize ImageManager::ImageRequest::size() const
{
    return QSize( m_width, m_height );
}

// vi:expandtab:tabstop=4 shiftwidth=4:
