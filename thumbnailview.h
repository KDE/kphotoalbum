#ifndef THUMBNAILVIEW_H
#define THUMBNAILVIEW_H
#include <qiconview.h>
#include "imageinfo.h"
class ImageManager;

class ThumbNailView :public QIconView {
    Q_OBJECT

public:
    ThumbNailView( QWidget* parent,  const char* name = 0 );
    void load( ImageInfoList* list );

public slots:
    void reload();
    void slotSelectAll();

protected slots:
    void showImage( QIconViewItem* );
    virtual void startDrag();
private:
    ImageInfoList* _imageList;
};

#endif /* THUMBNAILVIEW_H */

