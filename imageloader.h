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
class ImageManager;
class QWaitCondition;
class ImageRequest;

class ImageLoader :public QThread {
public:
    ImageLoader( QWaitCondition* sleeper );
    static QImage rotateAndScale( QImage, int width, int height, int angle );
    static void removeThumbnail( const QString& imageFile );

protected:
    virtual void run();
    QImage tryLoadThumbnail( ImageRequest* request, bool& ok );
    QImage loadImage( ImageRequest* request, bool& ok );
    void writeThumbnail( ImageRequest* request, QImage image );
    int calcLoadSize( ImageRequest* request );
    QImage scaleAndRotate( ImageRequest* request, QImage img );
    QString thumbnailPath( ImageRequest* request );
    QString thumbnailPath( QString uri, int dim );
    QString requestURL( ImageRequest* request );

private:
    QWaitCondition* _sleeper;
};

#endif /* IMAGELOADER_H */

