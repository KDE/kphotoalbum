#ifndef VIEWER_H
#define VIEWER_H

#include <qdialog.h>
#include "imageclient.h"
#include "imageinfo.h"
class ImageInfo;
class QLabel;

class Viewer :public QDialog,  public ImageClient
{
    Q_OBJECT
public:
    static Viewer* instance();
    void load( const ImageInfoList& list, int index = 0 );
    virtual void pixmapLoaded( const QString& fileName, int width, int height, int angle, const QPixmap& );
    virtual void show();

protected:
    virtual void keyPressEvent( QKeyEvent* e );
    void load();

private:
    Viewer( QWidget* parent, const char* name = 0 );
    static Viewer* _instance;

    QLabel* _label;
    ImageInfoList _list;
    int _current;
    ImageInfo _info;
};

#endif /* VIEWER_H */

