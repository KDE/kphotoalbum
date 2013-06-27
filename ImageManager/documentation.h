/* Copyright (C) 2012 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef IMAGEMANAGER_DOCUMENTATION
#define IMAGEMANAGER_DOCUMENTATION
/**
  \namespace ImageManager
  \brief Manages creation and caching of thumbnails, as well as (raw) image decoding.

  <h2>Loading Thumbnails</h2>

  To use the \ref AsyncLoader for loading an Image, the receiving class must implement
  the \ref ImageClientInterface interface. When a widget needs a preview image for a
  file, it typically performs the following steps:
   -# Create an \ref ImageRequest.
   -# Adjust the \ref Priority for the request (optional).
   -# Queue the request with the \ref AsyncLoader.
   -# The \ref AsyncLoader then processes the request according to its priority,
  and eventually calls either the \ref ImageClientInterface::pixmapLoaded or the
  \ref ImageClientInterface::requestCanceled method.

  <h3>Example</h3>

       void Foo::requestPreview( ImageInfoPtr info ) {
           ImageManager::ImageRequest* request = new ImageManager::ImageRequest( info->fileName(), QSize(256,256), info->angle(), this );
           request->setPriority( ImageManager::Viewer );
           ImageManager::AsyncLoader::instance()->load( request );
       }

       void Foo::pixmapLoaded( const DB::FileName& fileName, const QSize& imgSize, const QSize& fullSize,
                               int angle, const QImage& img, const bool loadedOK ) {
            if (loadedOK)
                 useResultImage();
       } //


**/

#endif //  IMAGEMANAGER_DOCUMENTATION
// vi:expandtab:tabstop=4 shiftwidth=4:
