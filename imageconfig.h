#ifndef IMAGECONFIG_H
#define IMAGECONFIG_H
#include "imageinfo.h"
#include "imageconfigui.h"
#include "imageclient.h"

class ImageConfig :public ImageConfigUI, public ImageClient {
public:
    ImageConfig( QWidget* parent, const char* name = 0 );
    void setImageInfo( ImageInfoList list );
    virtual void pixmapLoaded( const QString&, int, int, const QPixmap& );

protected:
    void slotRevert();
    void slotPrev();
    void slotNext();
    void slotDone();
    void load();
    void save();

private:
    ImageInfoList _list;
    int _current;
    QMap<QString, QPixmap> _preloadImageMap;
};

#endif /* IMAGECONFIG_H */

