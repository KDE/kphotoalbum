#include "myimageinfo.h"
MyImageInfo::MyImageInfo( const KURL& url )
    : KIPI::ImageInfoShared( url )
{
}

QString MyImageInfo::name()
{
    return QString::fromLatin1("a name"); // PENDING(blackie) implement
}

QString MyImageInfo::description()
{
    return QString::fromLatin1("a name"); // PENDING(blackie) implement
}

QMap<QString,QVariant> MyImageInfo::attributes()
{
    return QMap<QString,QVariant>(); // PENDING(blackie) implement
}

