#ifndef THUMBNAILBUILDER_H
#define THUMBNAILBUILDER_H
#include "imageclient.h"
#include <qprogressdialog.h>
#include <qimage.h>
#include "imageinfo.h"

class ThumbnailBuilder :public QProgressDialog, public ImageClient {
    Q_OBJECT

public:
    ThumbnailBuilder( QWidget* parent, const char* name = 0 );
    void generateNext();
    virtual void pixmapLoaded( const QString& fileName, const QSize& size, const QSize& fullSize, int angle, const QImage& );

private:
    ImageInfoList _images;
    uint _index;
};


#endif /* THUMBNAILBUILDER_H */

