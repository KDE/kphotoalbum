/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <config.h>
#ifdef HASKIPI
#include "myimageinfo.h"
#include "imagedb.h"
#include "imageinfo.h"
#include <kdebug.h>
MyImageInfo::MyImageInfo( KIPI::Interface* interface, const KURL& url )
    : KIPI::ImageInfoShared( interface, url )
{
    _info = ImageDB::instance()->info( _url.path() );
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
            return _info->date().start() ;
        }
        else
            return _info->date().end();
    }
    else
        return KIPI::ImageInfoShared::time( what );
}

bool MyImageInfo::isTimeExact()
{
    if ( !_info )
        return true;
    return _info->date().hasValidTime();
}

void MyImageInfo::setTime( const QDateTime& time, KIPI::TimeSpec spec )
{
    if ( !_info )
        return;
    if ( spec == KIPI::FromInfo ) {
        _info->setDate( ImageDate( time, time ) );
    }
    else {
        ImageDate date = _info->date();
        _info->setDate( ImageDate( date.start(), time ) );
    }
}

void MyImageInfo::cloneData( ImageInfoShared* other )
{
    ImageInfoShared::cloneData( other );
    if ( _info ) {
        MyImageInfo* inf = static_cast<MyImageInfo*>( other );
        _info->setDate( inf->_info->date() );
    }
}

#endif // KIPI
