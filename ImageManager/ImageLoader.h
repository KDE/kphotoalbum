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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef IMAGELOADER_H
#define IMAGELOADER_H
#include <qthread.h>
class QWaitCondition;

namespace ImageManager
{
class Manager;
class ImageRequest;

class ImageLoader :public QThread {
public:
    ImageLoader( QWaitCondition* sleeper );
    static QImage rotateAndScale( QImage, int width, int height, int angle );
    static void removeThumbnail( const QString& imageFile );
    static QImage tryLoadThumbnail( ImageRequest* request, bool& ok );
    static void writeThumbnail( ImageRequest* request, QImage image );

protected:
    virtual void run();
    QImage loadImage( ImageRequest* request, bool& ok );
    static int calcLoadSize( ImageRequest* request );
    QImage scaleAndRotate( ImageRequest* request, QImage img );
    static QString thumbnailPath( ImageRequest* request );
    static QString thumbnailPath( QString uri, int dim );
    static QString requestURL( ImageRequest* request );
    bool shouldImageBeScale( const QImage& img, ImageRequest* request );

private:
    QWaitCondition* _sleeper;
};

}

#endif /* IMAGELOADER_H */

