#ifndef VIEWER_H
#define VIEWER_H

#include <qdialog.h>
#include "imageclient.h"
#include "imageinfo.h"
#include "options.h"
class ImageInfo;
class QLabel;
class QPopupMenu;
class QAction;

class Viewer :public QDialog,  public ImageClient
{
    Q_OBJECT
public:
    static Viewer* instance( QWidget* parent = 0 );
    ~Viewer();
    void load( const ImageInfoList& list, int index = 0 );
    virtual void pixmapLoaded( const QString& fileName, int width, int height, int angle, const QPixmap& );
    virtual void show();

protected:
    virtual void mousePressEvent( QMouseEvent* e );
    virtual void mouseMoveEvent( QMouseEvent* e );
    virtual void mouseReleaseEvent( QMouseEvent* );
    virtual void contextMenuEvent ( QContextMenuEvent * e );

    void load();
    void saveOptions();
    void setDisplayedPixmap();

protected slots:
    void showNext();
    void showPrev();
    void showFirst();
    void showLast();
    void zoomIn();
    void zoomOut();
    void rotate90();
    void rotate180();
    void rotate270();
    void toggleShowInfoBox( bool );
    void toggleShowDescription( bool );
    void toggleShowDate( bool );
    void toggleShowNames( bool );
    void toggleShowLocation( bool );
    void toggleShowKeyWords( bool );

private:
    Viewer( QWidget* parent, const char* name = 0 );
    static Viewer* _instance;

    QAction* _firstAction;
    QAction* _lastAction;
    QAction* _nextAction;
    QAction* _prevAction;

    QLabel* _label;
    ImageInfoList _list;
    int _current;
    ImageInfo _info;
    QPixmap _pixmap;
    bool _moving;
    QRect _textRect;
    QPopupMenu* _popup;
    int _width, _height;
    Options::Position _startPos;
};

#endif /* VIEWER_H */

