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

#ifndef IMAGEPREVIEW_H
#define IMAGEPREVIEW_H
#include <qlabel.h>
#include "DB/ImageInfo.h"
#include "ImageManager/ImageClient.h"

namespace AnnotationDialog
{

class ImagePreview :public QLabel, public ImageManager::ImageClient {
    Q_OBJECT
public:
    ImagePreview( QWidget* parent, const char* name = 0);
    virtual QSize sizeHint() const;
    void rotate(int angle);
    void setImage( const DB::ImageInfo& info );
    void setImage( const QString& fileName );
    int angle() const;
    void anticipate(DB::ImageInfo &info1);
    virtual void pixmapLoaded( const QString& fileName, const QSize& size, const QSize& fullSize, int angle, const QImage&, bool loadedOK );

protected:
    virtual void resizeEvent( QResizeEvent* );
    void reload();
    void setCurrentImage(const QImage &image);

    class PreviewImage {
    public:
        bool has(const QString &fileName) const;
        QImage &getImage();
        const QString &getName() const;
        void set(const QString &fileName, const QImage &image);
        void set(const PreviewImage &other);
        void reset();
    protected:
        QString _fileName;
        QImage _image;
    };

    struct PreloadInfo {
        PreloadInfo();
        void set(const QString& fileName, int angle);
        QString _fileName;
        int _angle;
    };

    class PreviewLoader : public ImageManager::ImageClient, public PreviewImage  {
    public:
        void preloadImage( const QString& fileName, int width, int height, int angle);
        void cancelPreload();
        virtual void pixmapLoaded( const QString& fileName, const QSize& size, const QSize& fullSize, int angle, const QImage&, bool loadedOK );
    };
    PreviewLoader _preloader;

private:
    DB::ImageInfo _info;
    QString _fileName;
    PreviewImage _currentImage, _lastImage;
    PreloadInfo _anticipated;
    int _angle;
};

}


#endif /* IMAGEPREVIEW_H */

