#ifndef IMAGECONFIG_H
#define IMAGECONFIG_H
#include "imageinfo.h"
#include "imageconfigui.h"
#include "imageclient.h"
#include "listselect.h"

class ImageConfig :public ImageConfigUI, public ImageClient {
    Q_OBJECT
public:
    ImageConfig( QWidget* parent, const char* name = 0 );
    virtual void pixmapLoaded( const QString&, int, int, int, const QPixmap& );
    int configure( ImageInfoList list,  bool oneAtATime );
    int search();
    bool match( ImageInfo* info );

protected slots:
    void displayImage();

protected:
    enum SetupType { SINGLE, MULTIPLE, SEARCH };

    void slotRevert();
    void slotPrev();
    void slotNext();
    void slotOK();
    void load();
    void save();
    void setup();

private:
    ImageInfoList _origList;
    QValueList<ImageInfo> _editList;
    int _current;
    QMap<QString, QPixmap> _preloadImageMap;
    SetupType _setup;

    QPtrList< ListSelect > _optionList;
};

#endif /* IMAGECONFIG_H */

