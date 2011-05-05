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

#ifndef DISPLAYAREA_H
#define DISPLAYAREA_H
#include <qpixmap.h>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include "ImageManager/ImageClient.h"
#include <qimage.h>
#include <q3ptrvector.h>
#include "DB/ImageInfoPtr.h"
#include "Display.h"
#include "Settings/SettingsData.h"

class QTimer;

namespace DB
{
    class ImageInfo;
}

namespace Viewer
{
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
    ImageDisplay( QWidget* parent );
    bool setImage( DB::ImageInfoPtr info, bool forward );
    QImage currentViewAsThumbnail() const;
    virtual void pixmapLoaded( const QString& fileName, const QSize& size, const QSize& fullSize, int angle, const QImage&, const bool loadedOK);
    void setImageList( const QStringList& list );

    void filterNone();
    void filterSelected();
    bool filterMono();
    bool filterBW();
    bool filterContrastStretch();
    bool filterHistogramEqualization();

public slots:
    void zoomIn();
    void zoomOut();
    void zoomFull();
    void zoomPixelForPixel();

protected slots:
    void hideCursor();
    void showCursor();
    void disableCursorHiding();
    void enableCursorHiding();

signals:
    void possibleChange();
    void imageReady();
    void setCaptionInfo(const QString& info);

protected:
    virtual void mousePressEvent( QMouseEvent* event );
    virtual void mouseMoveEvent( QMouseEvent* event );
    virtual void mouseReleaseEvent( QMouseEvent* event );
    virtual void resizeEvent( QResizeEvent* event );
    virtual void paintEvent( QPaintEvent* event );
    OVERRIDE void hideEvent(QHideEvent* );
    QPoint mapPos( QPoint );
    QPoint offset( int logicalWidth, int logicalHeight, int physicalWidth, int physicalHeight, double* ratio );
    void xformPainter( QPainter* );
    void cropAndScale();
    void updatePreload();
    int indexOf( const QString& fileName );
    void requestImage( const DB::ImageInfoPtr& info, bool priority = false );

    /** display zoom factor in title of display window */
    void updateZoomCaption();

    friend class ViewHandler;
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

    ViewHandler* _viewHandler;

    // zoom points in the coordinate system of the image.
    QPoint _zStart;
    QPoint _zEnd;

    Q3PtrVector<ViewPreloadInfo> _cache;
    QStringList _imageList;
    QMap<QString, DB::ImageInfoPtr> _loadMap;
    bool _reloadImageInProgress;
    int _forward;
    int _curIndex;
    bool _busy;
    ViewerWidget *_viewer;

    QTimer* _cursorTimer;
    bool _cursorHiding;
};

}


#endif /* DISPLAYAREA_H */

