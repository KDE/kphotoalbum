#ifndef MYIMAGEINFO_H
#define MYIMAGEINFO_H

#include <libkipi/imageinfoshared.h>
#include "imageinfo.h"

class MyImageInfo :public KIPI::ImageInfoShared
{
public:
    MyImageInfo( const KURL& url );
    virtual QString name();
    virtual void setName( const QString& );

    virtual QString description();
    virtual void setDescription( const QString& );

    virtual QMap<QString,QVariant> attributes();
    virtual void clearAttributes();
    virtual void addAttributes( const QMap<QString,QVariant>& );
private:
    ImageInfo* _info;
};

#endif /* MYIMAGEINFO_H */

