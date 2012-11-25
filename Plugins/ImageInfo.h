/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef MYIMAGEINFO_H
#define MYIMAGEINFO_H

#include <config-kpa-kipi.h>
#include <libkipi/version.h>
#include <libkipi/imageinfoshared.h>
#include <kdemacros.h>
#include "DB/ImageInfoPtr.h"

namespace DB
{
    class ImageInfo;
}

namespace Plugins
{

class KDE_EXPORT ImageInfo :public KIPI::ImageInfoShared
{
public:
    ImageInfo( KIPI::Interface* interface, const KUrl& url );

#if KIPI_VERSION < 0x010500
    // kipi 1.5.0 uses attributes()/addAttributes() instead of these:
#if KIPI_VERSION >= 0x010300
    QString   name();
    void      setName( const QString& );
#else
    // "title" means filename
    QString title();
    void setTitle( const QString& );
#endif

    QString description();
    void setDescription( const QString& );

    int angle();
    void setAngle( int );

    QDateTime time( KIPI::TimeSpec what );
    void setTime( const QDateTime& time, KIPI::TimeSpec spec );
    bool isTimeExact();
#endif // KIPI_VERSION < 0x010500

    QMap<QString,QVariant> attributes();
    void clearAttributes();
    void addAttributes( const QMap<QString,QVariant>& );
    void delAttributes( const QStringList& );

#if KIPI_VERSION >= 0x010200
    void cloneData( ImageInfoShared* const other);
#else
    void cloneData( ImageInfoShared* other);
#endif // KIPI_VERSION >= 0x010200


private:
    DB::ImageInfoPtr _info;

    bool isPositionAttribute(const QString &key);
    bool isCategoryAttribute(const QString &key);
};

}

#endif /* MYIMAGEINFO_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
