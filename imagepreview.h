#ifndef IMAGEPREVIEW_H
#define IMAGEPREVIEW_H
#include <qlabel.h>
#include "imageinfo.h"
#include "imageclient.h"
#include <qimage.h>

class ImagePreview :public QLabel, public ImageClient {
    Q_OBJECT
public:
    ImagePreview( QWidget* parent, const char* name = 0);
    void setInfo( ImageInfo* info );
    virtual void pixmapLoaded( const QString& fileName, int width, int height, int angle, const QImage& );
    virtual QSize sizeHint() const;

signals:
    void doubleClicked();

protected:
    virtual void keyPressEvent( QKeyEvent* );
    virtual void resizeEvent( QResizeEvent* );
    void reload();

private:
    ImageInfo* _info;
    QImage _img;
};


#endif /* IMAGEPREVIEW_H */

