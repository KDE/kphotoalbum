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

