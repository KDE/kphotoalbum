#ifndef MYIMAGEINFO_H
#define MYIMAGEINFO_H

#include <libkipi/imageinfoshared.h>
#include "imageinfo.h"

class MyImageInfo :public KIPI::ImageInfoShared
{
public:
    MyImageInfo( KIPI::Interface* interface, const KURL& url );
    virtual QString title();
    virtual void setTitle( const QString& );

    virtual QString description();
    virtual void setDescription( const QString& );

    virtual QMap<QString,QVariant> attributes();
    virtual void clearAttributes();
    virtual void addAttributes( const QMap<QString,QVariant>& );

    virtual int angle();
    virtual void setAngle( int );

    virtual QDateTime time( KIPI::TimeSpec what );
    virtual void setTime( const QDateTime& time, KIPI::TimeSpec spec );
    virtual bool isTimeExact();

    virtual void cloneData( ImageInfoShared* other );

private:
    ImageInfo* _info;
};

#endif /* MYIMAGEINFO_H */

