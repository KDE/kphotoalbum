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

#ifndef DISPLAYAREA_H
#define DISPLAYAREA_H
#include <qlabel.h>
#include <qpixmap.h>
#include <qptrlist.h>
#include "drawlist.h"
#include "imageclient.h"
#include <qimage.h>
#include <qcache.h>
#include "imageinfo.h"
#include <qptrvector.h>
class Draw;
class ImageInfo;
class ViewHandler_viewHandler;
class DrawHandler;
class DisplayAreaHandler;
class ViewHandler;
class LoadInfo;

struct ViewPreloadInfo
{
    ViewPreloadInfo( const QImage& img, const QSize& size, int angle )
        : img(img), size(size), angle(angle) {}
    QImage img;
    QSize size;
    int angle;
};

class DisplayArea :public QWidget, public ImageClient {
Q_OBJECT
public:
    DisplayArea( QWidget* parent, const char* name = 0 );
    void startDrawing();
    void stopDrawing();
    void setImage( ImageInfo* info, bool forward );
    DrawHandler* drawHandler();
    QImage currentViewAsThumbnail() const;
    virtual void pixmapLoaded( const QString& fileName, const QSize& size, const QSize& fullSize, int angle, const QImage& );
    void setImageList( const ImageInfoList& list );

public slots:
    void toggleShowDrawings( bool );
    void zoomIn();
    void zoomOut();
    void zoomFull();

protected slots:
    void drawAll();
    void doShowDrawings();

signals:
    void possibleChange();

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

    friend class DrawHandler;
    friend class ViewHandler;
    QPainter* painter();
    void zoom( QPoint p1, QPoint p2 );
    void normalize( QPoint& p1, QPoint& p2 );
    void pan( const QPoint& );
    void busy();
    void unbusy();

private:
    QImage _loadedImage;
    QImage _croppedAndScaledImg;
    QPixmap _drawingPixmap;
    QPixmap _viewPixmap;
    ImageInfo* _info;

    ViewHandler* _viewHandler;
    DrawHandler* _drawHanler;
    DisplayAreaHandler* _currentHandler;

    QPoint _zStart; // Stands for zoom start
    QPoint _zEnd;
    QPtrVector<ViewPreloadInfo> _cache;
    ImageInfoList _imageList;
    bool _cachedView;
    bool _reloadImageInProgress;
    int _forward;
    int _curIndex;
    bool _busy;
};


#endif /* DISPLAYAREA_H */

