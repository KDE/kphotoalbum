#ifndef IMAGECONFIG_H
#define IMAGECONFIG_H
#include "imageinfo.h"
#include "imageconfigui.h"
#include "imageclient.h"
#include "listselect.h"
#include "imagesearchinfo.h"

class ImageConfig :public ImageConfigUI, public ImageClient {
    Q_OBJECT
public:
    ImageConfig( QWidget* parent, const char* name = 0 );
    virtual void pixmapLoaded( const QString&, int, int, int, const QPixmap& );
    int configure( ImageInfoList list,  bool oneAtATime );
    int search();
    bool match( ImageInfo* info );

signals:
    void changed();

protected slots:
    void displayImage();
    void slotRevert();
    void slotPrev();
    void slotNext();
    void slotOK();
    void slotClear();

protected:
    enum SetupType { SINGLE, MULTIPLE, SEARCH };
    void load();
    void save();
    void setup();
    void loadInfo( const ImageSearchInfo& );

private:
    ImageInfoList _origList;
    QValueList<ImageInfo> _editList;
    int _current;
    // PENDING(blackie) We can't just have a QMap as this fills up memory.
    // QMap<QString, QPixmap> _preloadImageMap;
    SetupType _setup;
    QPtrList< ListSelect > _optionList;
    ImageSearchInfo _oldSearch;
};

#endif /* IMAGECONFIG_H */

