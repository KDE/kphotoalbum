#ifndef MYIMAGECOLLECTION_H
#define MYIMAGECOLLECTION_H

#include <libkipi/imagecollectionshared.h>
#include "imageinfo.h"

class MyImageCollection :public KIPI::ImageCollectionShared
{
public:
    enum Type { CurrentAlbum, CurrentSelection, SubClass };

    MyImageCollection( Type tp );
    virtual QString name();
    virtual QString comment();
    virtual KURL::List images();
    virtual KURL path();
    virtual KURL uploadPath();
    virtual KURL uploadRoot();

protected:
    KURL::List imageListToUrlList( const ImageInfoList& list );
    KURL commonRoot();

private:
    Type _tp;
};

#endif /* MYIMAGECOLLECTION_H */

