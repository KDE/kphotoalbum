#include "myimageinfo.h"
#include "imagedb.h"
MyImageInfo::MyImageInfo( const KURL& url )
    : KIPI::ImageInfoShared( url )
{
    _info = ImageDB::instance()->find( _url.path() );
}

QString MyImageInfo::name()
{
    if ( _info )
        return _info->label();
    else
        return QString::null;
}

QString MyImageInfo::description()
{
    if ( _info )
        return _info->description();
    else
        return QString::null;
}

QMap<QString,QVariant> MyImageInfo::attributes()
{
    return QMap<QString,QVariant>(); // PENDING(blackie) implement
}

void MyImageInfo::setName( const QString& name )
{
    if ( _info )
        _info->setLabel( name );
}

void MyImageInfo::setDescription( const QString& description )
{
    if ( _info )
        _info->setDescription( description );
}

void MyImageInfo::clearAttributes()
{
    // PENDING(blackie) implement
    qDebug("NYI: MyImageInfo::clearAttributes" );
}

void MyImageInfo::addAttributes( const QMap<QString,QVariant>& /* map */ )
{
    // PENDING(blackie) implement
    qDebug("NYI: MyImageInfo::addAttributes" );
}

