#ifndef IMAGECONFIG_H
#define IMAGECONFIG_H
#include "imageinfo.h"
#include "imageconfigui.h"
#include "imageclient.h"
#include "listselect.h"

class ImageConfig :public ImageConfigUI, public ImageClient {
public:
    ImageConfig( QWidget* parent, const char* name = 0 );
    virtual void pixmapLoaded( const QString&, int, int, int, const QPixmap& );
    int exec( ImageInfoList list,  bool oneAtATime );

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
    bool _oneAtATime;

    QPtrList< ListSelect > _optionList;
};

#endif /* IMAGECONFIG_H */

