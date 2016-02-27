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


    QMap<QString,QVariant> attributes();
    void clearAttributes();
    void addAttributes( const QMap<QString,QVariant>& );
    void delAttributes( const QStringList& );

    void cloneData( ImageInfoShared* const other);


private:
    DB::ImageInfoPtr m_info;

    bool isPositionAttribute(const QString &key);
    bool isCategoryAttribute(const QString &key);
};

}

#endif /* MYIMAGEINFO_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
