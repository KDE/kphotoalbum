#ifndef MYIMAGECOLLECTION_H
#define MYIMAGECOLLECTION_H
#ifdef HAVE_KIPI

#include <libkipi/imagecollectionshared.h>
#include "imageinfo.h"

class MyImageCollection :public KIPI::ImageCollectionShared
{
public:
    enum Type { CurrentAlbum, CurrentView, CurrentSelection };

    MyImageCollection( Type tp );
    virtual QString name();
    virtual QString comment();
    virtual KURL::List images();
    virtual KURL root();

protected:
    KURL::List imageListToUrlList( const ImageInfoList& list );

private:
    Type _tp;
};

#endif // HAVE_KIPI
#endif /* MYIMAGECOLLECTION_H */

