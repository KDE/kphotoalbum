/*
 *  Copyright (c) 2003-2004 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef IMAGEPREVIEW_H
#define IMAGEPREVIEW_H
#include <qlabel.h>
#include "imageinfo.h"
#include "imageclient.h"
#include <qimage.h>

class ImagePreview :public QLabel, public ImageClient {
    Q_OBJECT
public:
    ImagePreview( QWidget* parent, const char* name = 0);
    virtual QSize sizeHint() const;
    void rotate(int angle);
    void setImage( const ImageInfo& info );
    void setImage( const QString& fileName );
    int angle() const;
    void anticipate(ImageInfo &info1);
    virtual void pixmapLoaded( const QString& fileName, int width, int height, int angle, const QImage& );

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
    
    class PreviewLoader : public ImageClient, public PreviewImage  {
    public:
        void preloadImage( const QString& fileName, int width, int height, int angle);
        void cancelPreload();    
        virtual void pixmapLoaded( const QString& fileName, int width, int height, int angle, const QImage& );
    };
    PreviewLoader _preloader;

private:
    ImageInfo _info;
    QString _fileName;
    PreviewImage _currentImage, _lastImage;
    PreloadInfo _anticipated;
    int _angle;
};


#endif /* IMAGEPREVIEW_H */

