/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef DISPLAYAREA_H
#define DISPLAYAREA_H
#include <qpixmap.h>
#include "ImageManager/ImageClient.h"
#include <qimage.h>
#include <qptrvector.h>
#include "DB/ImageInfoPtr.h"
#include "Display.h"
#include "Settings/SettingsData.h"

namespace DB
{
    class ImageInfo;
}

namespace ImageManager
{
    class ImageRequest;
}

namespace Viewer
{
class Draw;
class DrawHandler;
class DisplayAreaHandler;
class ViewHandler;
class ViewerWidget;

struct ViewPreloadInfo
{
    ViewPreloadInfo( const QImage& img, const QSize& size, int angle )
        : img(img), size(size), angle(angle) {}
    QImage img;
    QSize size;
    int angle;
};

class ImageDisplay :public Viewer::Display, public ImageManager::ImageClient {
Q_OBJECT
public:
    ImageDisplay( QWidget* parent, const char* name = 0 );
    void startDrawing();
    void stopDrawing();
    bool setImage( DB::ImageInfoPtr info, bool forward );
    DrawHandler* drawHandler();
    QImage currentViewAsThumbnail() const;
    virtual void pixmapLoaded( const QString& fileName, const QSize& size, const QSize& fullSize, int angle, const QImage&, bool loadedOK );
    void setImageList( const QStringList& list );
    virtual bool offersDrawOnImage() const { return true; }

public slots:
    void toggleShowDrawings( bool );
    void zoomIn();
    void zoomOut();
    void zoomFull();
    void zoomPixelForPixel();

protected slots:
    void drawAll();
    void doShowDrawings();

signals:
    void possibleChange();
    void setCaptionInfo(const QString& info);

protected:
    virtual void mousePressEvent( QMouseEvent* event );
    virtual void mouseMoveEvent( QMouseEvent* event );
    virtual void mouseReleaseEvent( QMouseEvent* event );
    virtual void resizeEvent( QResizeEvent* event );
    virtual void paintEvent( QPaintEvent* event );
    QPoint mapPos( QPoint );
    QPoint offset( int logicalWidth, int logicalHeight, int physicalWidth, int physicalHeight, double* ratio );
    void xformPainter( QPainter* );
    void cropAndScale();
    void updatePreload();
    int indexOf( const QString& fileName );
    void requestImage( const DB::ImageInfoPtr& info );

    /** display zoom factor in title of display window */
    void updateZoomCaption();

    friend class DrawHandler;
    friend class ViewHandler;
    QPainter* painter();
    void zoom( QPoint p1, QPoint p2 );
    void normalize( QPoint& p1, QPoint& p2 );
    void pan( const QPoint& );
    void retryZoom();
    void busy();
    void unbusy();
    bool isImageZoomed( const Settings::StandardViewSize type, const QSize& imgSize );
    void updateZoomPoints( const Settings::StandardViewSize type, const QSize& imgSize );
    void potentialyLoadFullSize();
    double sizeRatio( const QSize& baseSize, const QSize& newSize ) const;

private:
    QImage _loadedImage;
    QImage _croppedAndScaledImg;
    QPixmap _drawingPixmap;
    QPixmap _viewPixmap;

    ViewHandler* _viewHandler;
    DrawHandler* _drawHandler;
    DisplayAreaHandler* _currentHandler;

    // zoom points in the coordinate system of the image.
    QPoint _zStart;
    QPoint _zEnd;

    QPtrVector<ViewPreloadInfo> _cache;
    QStringList _imageList;
    QMap<QString, DB::ImageInfoPtr> _loadMap;
    bool _reloadImageInProgress;
    int _forward;
    int _curIndex;
    bool _busy;
    ViewerWidget *_viewer;
};

}


#endif /* DISPLAYAREA_H */

