// krazy:skip

// SPDX-FileCopyrightText: 2009, 2012 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

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
