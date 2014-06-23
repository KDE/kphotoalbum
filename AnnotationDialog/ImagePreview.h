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
class QRubberBand;

namespace AnnotationDialog
{
class ResizableFrame;

class ImagePreview :public QLabel, public ImageManager::ImageClientInterface {
    Q_OBJECT
public:
    explicit ImagePreview( QWidget* parent );
    virtual QSize sizeHint() const;
    void rotate(int angle);
    void setImage( const DB::ImageInfo& info );
    void setImage( const QString& fileName );
    int angle() const;
    void anticipate(DB::ImageInfo &info1);
    void pixmapLoaded(ImageManager::ImageRequest* request, const QImage& image) override;
    QRect areaPreviewToActual(QRect area) const;
    QRect minMaxAreaPreview() const;
    void createTaggedArea(QString category, QString tag, QRect geometry, bool showArea);
    QSize getActualImageSize();

public slots:
    void setAreaCreationEnabled(bool state);

signals:
    void areaCreated(ResizableFrame *area);

protected:
    virtual void resizeEvent( QResizeEvent* );
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    void reload();
    void setCurrentImage(const QImage &image);
    QImage rotateAndScale( QImage, int width, int height, int angle ) const;

    QRect areaActualToPreview(QRect area) const;
    void processNewArea();
    void remapAreas();
    void rotateAreas(int angle);

    class PreviewImage {
    public:
        bool has(const DB::FileName &fileName, int angle) const;
        QImage &getImage();
        void set(const DB::FileName &fileName, const QImage &image, int angle);
        void set(const PreviewImage &other);
        void setAngle( int angle );
        void reset();
    protected:
        DB::FileName _fileName;
        QImage _image;
        int _angle;
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
        void pixmapLoaded(ImageManager::ImageRequest* request, const QImage& image) override;
    };
    PreviewLoader _preloader;

private:
    DB::ImageInfo _info;
    QString _fileName;
    PreviewImage _currentImage, _lastImage;
    PreloadInfo _anticipated;
    int _angle;
    int _minX;
    int _maxX;
    int _minY;
    int _maxY;
    QPoint _areaStart;
    QPoint _areaEnd;
    QPoint _currentPos;
    QRubberBand *_selectionRect;
    double _scaleWidth;
    double _scaleHeight;
    void createNewArea(QRect geometry, QRect actualGeometry);
    QRect rotateArea(QRect originalAreaGeometry, int angle);
    bool _areaCreationEnabled;
    QMap<QString, QPair<int, QSize>> _imageSizes;
};

}

#endif /* IMAGEPREVIEW_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
