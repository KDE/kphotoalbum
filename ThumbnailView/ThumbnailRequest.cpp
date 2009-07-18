/* Copyright (C) 2003-2009 Jesper K. Pedersen <blackie@kde.org>

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
#include "ThumbnailRequest.h"
#include "ThumbnailPainter.h"
#include "ThumbnailCache.h"

ThumbnailView::ThumbnailRequest::ThumbnailRequest( const QString& fileName, const QSize& size, int angle, ThumbnailPainter* client)
    : ImageManager::ImageRequest( fileName, size, angle, client ), _thumbnailPainter( client ), _fileName( fileName )
{
}

bool ThumbnailView::ThumbnailRequest::stillNeeded() const
{
    return _thumbnailPainter->thumbnailStillNeeded( _fileName );
}

ThumbnailView::ThumbnailCacheRequest::ThumbnailCacheRequest( const QString& fileName, const QSize& size, int angle, ThumbnailCache* client)
    : ImageManager::ImageRequest( fileName, size, angle, client ), _thumbnailCache( client ), _fileName( fileName )
{
}

bool ThumbnailView::ThumbnailCacheRequest::stillNeeded() const
{
    return _thumbnailCache->thumbnailStillNeeded( _fileName );
}
