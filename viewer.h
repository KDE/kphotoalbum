#ifndef VIEWER_H
#define VIEWER_H

#include "imageclient.h"
#include "imageinfo.h"
#include "options.h"
#include <kmainwindow.h>
#include <kaction.h>
class ImageInfo;
class QLabel;
class QPopupMenu;
class QAction;
class KToolBar;
class KAction;
class DisplayArea;

class Viewer :public KMainWindow,  public ImageClient
{
    Q_OBJECT
public:
    static Viewer* instance( QWidget* parent = 0 );
    ~Viewer();
    void load( const ImageInfoList& list, int index = 0 );
    virtual void pixmapLoaded( const QString& fileName, int width, int height, int angle, const QPixmap& );

protected:
    virtual void mousePressEvent( QMouseEvent* e );
    virtual void mouseMoveEvent( QMouseEvent* e );
    virtual void mouseReleaseEvent( QMouseEvent* );
    virtual void contextMenuEvent ( QContextMenuEvent * e );
    virtual void closeEvent( QCloseEvent* e );

    void load();
    void saveOptions();
    void setDisplayedPixmap();
    void setupContextMenu();

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
    void startDraw();
    void stopDraw();

private:
    Viewer( QWidget* parent, const char* name = 0 );
    static Viewer* _instance;

    QAction* _firstAction;
    QAction* _lastAction;
    QAction* _nextAction;
    QAction* _prevAction;

    DisplayArea* _label;
    ImageInfoList _list;
    int _current;
    ImageInfo _info;
    bool _moving;
    QRect _textRect;
    QPopupMenu* _popup;
    int _width, _height;
    Options::Position _startPos;
    QPixmap _pixmap;

    KToolBar* _toolbar;
    KToggleAction* _select;
    KToggleAction* _line;
    KToggleAction* _rect;
    KToggleAction* _circle;
    KAction* _delete;
};

#endif /* VIEWER_H */

