#ifndef MYIMAGEINFO_H
#define MYIMAGEINFO_H

#include <libkipi/imageinfoshared.h>

class MyImageInfo :public KIPI::ImageInfoShared
{
public:
    MyImageInfo( const KURL& url );
    virtual QString name();
    virtual QString description();
    virtual QMap<QString,QVariant> attributes();
};

#endif /* MYIMAGEINFO_H */

