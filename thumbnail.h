#ifndef THUMBNAIL_H
#define THUMBNAIL_H

#include <qlabel.h>
#include <qiconview.h>
#include "imageclient.h"
class ThumbNailView;
class ImageInfo;

class ThumbNail :public QIconViewItem, public ImageClient {
public:
    friend class ThumbNailView;
    ThumbNail( ImageInfo* imageInfo,  ThumbNailView* parent,  const char* name = 0);
    ThumbNail( ImageInfo* imageInfo,  ThumbNail* after, ThumbNailView* parent,  const char* name = 0);

    virtual void pixmapLoaded( const QString&, int, int, int, const QPixmap& pixmap );
    QString fileName() const;
    ImageInfo* imageInfo();
    virtual bool acceptDrop ( const QMimeSource * mime ) const;

protected:
    void init();
    virtual void dragLeft();
    virtual void dragMove();
    virtual void dropped ( QDropEvent * e, const QValueList<QIconDragItem> & lst );
    bool atRightSizeOfItem();

private:
    ImageInfo* _imageInfo;
    QPixmap _pixmap;
    ThumbNailView* _parent;
};

#endif /* THUMBNAIL_H */

