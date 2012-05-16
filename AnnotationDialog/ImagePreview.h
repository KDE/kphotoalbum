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

#ifndef IMAGEPREVIEW_H
#define IMAGEPREVIEW_H
#include <QLabel>
#include "DB/ImageInfo.h"
#include "ImageManager/ImageClientInterface.h"
class QResizeEvent;

namespace AnnotationDialog
{

class ImagePreview :public QLabel, public ImageManager::ImageClientInterface {
    Q_OBJECT
public:
    ImagePreview( QWidget* parent );
    virtual QSize sizeHint() const;
    void rotate(int angle);
    void setImage( const DB::ImageInfo& info );
    void setImage( const QString& fileName );
    int angle() const;
    void anticipate(DB::ImageInfo &info1);
    OVERRIDE void pixmapLoaded( const DB::FileName& fileName, const QSize& size, const QSize& fullSize, int angle, const QImage&, const bool loadedOK);

protected:
    virtual void resizeEvent( QResizeEvent* );
    void reload();
    void setCurrentImage(const QImage &image);
    QImage rotateAndScale( QImage, int width, int height, int angle ) const;


    class PreviewImage {
    public:
        bool has(const DB::FileName &fileName) const;
        QImage &getImage();
        void set(const DB::FileName &fileName, const QImage &image);
        void set(const PreviewImage &other);
        void reset();
    protected:
        DB::FileName _fileName;
        QImage _image;
    };

    struct PreloadInfo {
        PreloadInfo();
        void set(const DB::FileName& fileName, int angle);
        DB::FileName _fileName;
        int _angle;
    };

    class PreviewLoader : public ImageManager::ImageClientInterface, public PreviewImage  {
    public:
        void preloadImage( const DB::FileName& fileName, int width, int height, int angle);
        void cancelPreload();
        virtual void pixmapLoaded( const DB::FileName& fileName, const QSize& size, const QSize& fullSize, int angle, const QImage&, const bool loadedOK);
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

