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

#ifndef IMAGEDISPLAY_H
#define IMAGEDISPLAY_H

#include <qpixmap.h>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include "ImageManager/ImageClientInterface.h"
#include <qimage.h>
#include "DB/ImageInfoPtr.h"
#include "AbstractDisplay.h"
#include "Settings/SettingsData.h"
#include <DB/FileNameList.h>
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
    ViewPreloadInfo()  {}
    ViewPreloadInfo( const QImage& img, const QSize& size, int angle )
        : img(img), size(size), angle(angle) {}
    QImage img;
    QSize size;
    int angle;
};

class ImageDisplay :public Viewer::AbstractDisplay, public ImageManager::ImageClientInterface {
Q_OBJECT
public:
    explicit ImageDisplay( QWidget* parent );
    bool setImage( DB::ImageInfoPtr info, bool forward ) override;
    QImage currentViewAsThumbnail() const;
    void pixmapLoaded(ImageManager::ImageRequest* request, const QImage& image) override;
    void setImageList( const DB::FileNameList& list );

    void filterNone();
    void filterSelected();
    bool filterMono();
    bool filterBW();
    bool filterContrastStretch();
    bool filterHistogramEqualization();

public slots:
    void zoomIn() override;
    void zoomOut() override;
    void zoomFull() override;
    void zoomPixelForPixel() override;

protected slots:
    void hideCursor();
    void showCursor();
    void disableCursorHiding();
    void enableCursorHiding();

signals:
    void possibleChange();
    void imageReady();
    void setCaptionInfo(const QString& info);
    void viewGeometryChanged(QSize viewSize, QRect zoomWindow, double sizeRatio);

protected:
    virtual void mousePressEvent( QMouseEvent* event ) override;
    virtual void mouseMoveEvent( QMouseEvent* event ) override;
    virtual void mouseReleaseEvent( QMouseEvent* event ) override;
    virtual void resizeEvent( QResizeEvent* event ) override;
    virtual void paintEvent( QPaintEvent* event ) override;
    void hideEvent(QHideEvent* ) override;
    QPoint mapPos( QPoint );
    QPoint offset( int logicalWidth, int logicalHeight, int physicalWidth, int physicalHeight, double* ratio );
    void xformPainter( QPainter* );
    void cropAndScale();
    void updatePreload();
    int indexOf( const DB::FileName& fileName );
    void requestImage( const DB::ImageInfoPtr& info, bool priority = false );

    /** display zoom factor in title of display window */
    void updateZoomCaption();

    friend class ViewHandler;
    void zoom( QPoint p1, QPoint p2 );
    void normalize( QPoint& p1, QPoint& p2 );
    void pan( const QPoint& );
    void busy();
    void unbusy();
    bool isImageZoomed( const Settings::StandardViewSize type, const QSize& imgSize );
    void updateZoomPoints( const Settings::StandardViewSize type, const QSize& imgSize );
    void potentiallyLoadFullSize();
    double sizeRatio( const QSize& baseSize, const QSize& newSize ) const;

private:
    QImage m_loadedImage;
    QImage m_croppedAndScaledImg;

    ViewHandler* m_viewHandler;

    // zoom points in the coordinate system of the image.
    QPoint m_zStart;
    QPoint m_zEnd;

    QMap<int,ViewPreloadInfo> m_cache;
    DB::FileNameList m_imageList;
    QMap<QString, DB::ImageInfoPtr> m_loadMap;
    bool m_reloadImageInProgress;
    int m_forward;
    int m_curIndex;
    bool m_busy;
    ViewerWidget* m_viewer;

    QTimer* m_cursorTimer;
    bool m_cursorHiding;
};

}


#endif /* IMAGEDISPLAY_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
