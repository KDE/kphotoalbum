#ifndef EXTERNALPOPUP_H
#define EXTERNALPOPUP_H
#include <qpopupmenu.h>
#include "imageinfo.h"

class ExternalPopup :public QPopupMenu {
    Q_OBJECT

public:
    ExternalPopup( QWidget* parent, const char* name = 0 );
    void populate( ImageInfo* current, const ImageInfoList& list );

protected slots:
    void slotExecuteService( int );

private:
    ImageInfoList _list;
    ImageInfo* _currentInfo;
};


#endif /* EXTERNALPOPUP_H */

