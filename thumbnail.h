#ifndef THUMBNAIL_H
#define THUMBNAIL_H

#include <qlabel.h>
#include <qiconview.h>
#include "imageclient.h"
class ThumbNailView;
class ImageInfo;

class ThumbNail :public QIconViewItem, public ImageClient {
public:
    ThumbNail( ImageInfo* image,  ThumbNailView* parent,  const char* name );

    virtual void pixmapLoaded( const QString&, int, int, int, const QPixmap& pixmap );
    QString fileName() const;
    ImageInfo* imageInfo();

private:
    ImageInfo* _imageInfo;
};

#endif /* THUMBNAIL_H */

