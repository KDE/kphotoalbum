#ifndef VIEWER_H
#define VIEWER_H

#include <qdialog.h>
#include "imageclient.h"
class ImageInfo;
class QLabel;

class Viewer :public QDialog,  public ImageClient
{
public:
    Viewer( QWidget* parent, const char* name = 0 );
    void load( ImageInfo* );
    virtual void pixmapLoaded( const QString& fileName, int width, int height, const QPixmap& );

protected:
    virtual void resizeEvent( QResizeEvent* e );

private:
    QLabel* _label;
    ImageInfo* _info;
};

#endif /* VIEWER_H */

