#include "myimageinfo.h"
#include "imagedb.h"
MyImageInfo::MyImageInfo( KIPI::Interface* interface, const KURL& url )
    : KIPI::ImageInfoShared( interface, url )
{
    _info = ImageDB::instance()->find( _url.path() );
}

QString MyImageInfo::title()
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
    QMap<QString,QVariant> res;
    if ( _info ) {
        for( QMapIterator<QString,QStringList> it = _info->_options.begin(); it != _info->_options.end(); ++it ) {
            res.insert( it.key(), QVariant( it.data() ) );
        }
    }
    return res;
}

void MyImageInfo::setTitle( const QString& name )
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
    _info->_options.clear();
}

void MyImageInfo::addAttributes( const QMap<QString,QVariant>& map )
{
    if ( _info ) {
        for( QMapConstIterator<QString,QVariant> it = map.begin(); it != map.end(); ++it ) {
            QStringList list = it.data().toStringList();
            _info->addOption( it.key(), list );
        }
    }
}

int MyImageInfo::angle()
{
    if ( _info )
        return _info->angle();
    else
        return 0;
}

void MyImageInfo::setAngle( int angle )
{
    if ( _info )
        _info->setAngle( angle );
}

QDateTime MyImageInfo::time( KIPI::TimeSpec what )
{
    if ( _info ) {
        if ( what == KIPI::FromInfo ) {
            return QDateTime(_info->startDate().getDate(), _info->startDate().getTime() );
        }
        else
            return QDateTime(_info->endDate().getDate(), _info->endDate().getTime() );
    }
    else
        return KIPI::ImageInfoShared::time( what );
}

bool MyImageInfo::isTimeExact()
{
    ImageDate date = _info->endDate();
    if ( date.year() > 0 || date.month() > 0 || date.day() > 0 )
        return false;
    date = _info->startDate();
    if ( date.year() <= 0 || date.month() <= 0 || date.day() <= 0 )
        return false;
    return true;
}

void MyImageInfo::setTime( const QDateTime& time, KIPI::TimeSpec spec )
{
    if ( !_info )
        return;
    ImageDate& date = _info->startDate();
    if ( spec == KIPI::ToInfo )
        date = _info->endDate();

    date.setDate( time.date() );
    date.setTime( time.time() );
}

void MyImageInfo::cloneData( ImageInfoShared* other )
{
    ImageInfoShared::cloneData( other );
    if ( _info ) {
        MyImageInfo* inf = static_cast<MyImageInfo*>( other );
        _info->setStartDate( inf->_info->startDate() );
        _info->setEndDate( inf->_info->endDate() );
    }
}

