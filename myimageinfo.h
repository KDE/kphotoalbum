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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef MYIMAGEINFO_H
#define MYIMAGEINFO_H

#ifdef HASKIPI
#include <libkipi/imageinfoshared.h>
class ImageInfo;

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

#endif // NOKIP

#endif /* MYIMAGEINFO_H */

