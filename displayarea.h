/*
 *  Copyright (c) 2003 Jesper K. Pedersen <blackie@kde.org>
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
class Draw;
class ImageInfo;

class DisplayArea :public QLabel,  public ImageClient {
Q_OBJECT
public:
    DisplayArea( QWidget* parent, const char* name = 0 );
    DrawList drawList() const;
    void setDrawList( const DrawList& );
    void stopDrawings();
    void setImage( ImageInfo* info );

public slots:
    void slotLine();
    void slotRectangle();
    void slotCircle();
    void slotSelect();
    void setPixmap( const QPixmap& pixmap );
    void cut();
    void toggleShowDrawings( bool );

protected:
    virtual void mousePressEvent( QMouseEvent* event );
    virtual void mouseMoveEvent( QMouseEvent* event );
    virtual void mouseReleaseEvent( QMouseEvent* event );
    virtual void resizeEvent( QResizeEvent* event );
    Draw* createTool();
    void drawAll();
    Draw* findShape( const QPoint& );
    void setupPainter( QPainter& painter );
    void pixmapLoaded( const QString&, int, int, int, const QImage& image );

private:
    enum Tool { Select, Line, Rectangle, Circle, None};
    Tool _tool;
    Draw* _activeTool;
    DrawList _drawings;
    QPixmap _origPixmap;
    QPixmap _curPixmap;
    ImageInfo* _info;
    QImage _currentImage;
};


#endif /* DISPLAYAREA_H */

