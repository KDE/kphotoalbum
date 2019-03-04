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
#include "DB/ImageInfo.h"
#include "ImageManager/ImageClientInterface.h"

#include <QLabel>
#include <QTimer>

class QResizeEvent;
class QRubberBand;

namespace AnnotationDialog
{
class ResizableFrame;

class ImagePreview :public QLabel, public ImageManager::ImageClientInterface {
    Q_OBJECT
public:
    explicit ImagePreview( QWidget* parent );
    int heightForWidth( int width ) const override;
    QSize sizeHint() const override;
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
    void acceptProposedTag(QPair<QString, QString> tagData, ResizableFrame *area);
    QPixmap grabAreaImage(QRect area);

public slots:
    void setAreaCreationEnabled(bool state);

signals:
    void areaCreated(ResizableFrame *area);
    void proposedTagSelected(QString category, QString tag);

protected:
    void resizeEvent( QResizeEvent* ) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void reload();
    void setCurrentImage(const QImage &image);
    QImage rotateAndScale( QImage, int width, int height, int angle ) const;
    void updateScaleFactors();

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
        DB::FileName m_fileName;
        QImage m_image;
        int m_angle;
    };

    struct PreloadInfo {
        PreloadInfo();
        void set(const DB::FileName& fileName, int angle);
        DB::FileName m_fileName;
        int m_angle;
    };

    class PreviewLoader : public ImageManager::ImageClientInterface, public PreviewImage  {
    public:
        void preloadImage( const DB::FileName& fileName, int width, int height, int angle);
        void cancelPreload();
        void pixmapLoaded(ImageManager::ImageRequest* request, const QImage& image) override;
    };
    PreviewLoader m_preloader;

protected slots:
    void resizeFinished();
private:
    DB::ImageInfo m_info;
    QString m_fileName;
    PreviewImage m_currentImage, m_lastImage;
    PreloadInfo m_anticipated;
    int m_angle;
    int m_minX;
    int m_maxX;
    int m_minY;
    int m_maxY;
    QPoint m_areaStart;
    QPoint m_areaEnd;
    QPoint m_currentPos;
    QRubberBand *m_selectionRect;
    double m_scaleWidth;
    double m_scaleHeight;
    double m_aspectRatio;
    QTimer *m_reloadTimer;

    void createNewArea(QRect geometry, QRect actualGeometry);
    QRect rotateArea(QRect originalAreaGeometry, int angle);
    bool m_areaCreationEnabled;
    QMap<QString, QPair<int, QSize>> m_imageSizes;
    QImage m_fullSizeImage;
    void fetchFullSizeImage();
    bool fuzzyAreaExists(QList<QRect> &existingAreas, QRect area);
    float distance(QPoint point1, QPoint point2);
};

}

#endif /* IMAGEPREVIEW_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
