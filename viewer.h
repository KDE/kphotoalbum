#ifndef VIEWER_H
#define VIEWER_H

#include <qdialog.h>
#include "imageclient.h"
class ImageInfo;
class QLabel;

class Viewer :public QDialog,  public ImageClient
{
    Q_OBJECT
public:
    Viewer( QWidget* parent, const char* name = 0 );
    void load( ImageInfo* );
    virtual void pixmapLoaded( const QString& fileName, int width, int height, int angle, const QPixmap& );
    virtual void show();

protected slots:
    void resizeImage();

protected:
    virtual void resizeEvent( QResizeEvent* e );

private:
    QLabel* _label;
    ImageInfo* _info;
    QTimer* _timer;
};

#endif /* VIEWER_H */

