#ifndef IMAGEPREVIEW_H
#define IMAGEPREVIEW_H
#include <qlabel.h>
#include "imageinfo.h"
#include "imageclient.h"

class ImagePreview :public QLabel, public ImageClient {
public:
    ImagePreview( QWidget* parent, const char* name = 0);
    void setInfo( ImageInfo* info );
    virtual void pixmapLoaded( const QString& fileName, int width, int height, int angle, const QPixmap& );

protected:
    virtual void mouseDoubleClickEvent( QMouseEvent* );
    virtual void keyPressEvent( QKeyEvent* );
    void reload();

private:
    ImageInfo* _info;
};


#endif /* IMAGEPREVIEW_H */

