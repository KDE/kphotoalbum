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

#include <DB/ImageInfoPtr.h>

#include <KIPI/ImageInfoShared>
#include <config-kpa-kipi.h>

namespace DB
{
class ImageInfo;
}

namespace Plugins
{

class ImageInfo : public KIPI::ImageInfoShared
{
public:
    ImageInfo(KIPI::Interface *interface, const QUrl &url);

    QMap<QString, QVariant> attributes() override;
    void clearAttributes() override;
    void addAttributes(const QMap<QString, QVariant> &) override;
    void delAttributes(const QStringList &) override;

    void cloneData(ImageInfoShared *const other) override;

private:
    DB::ImageInfoPtr m_info;

    bool isPositionAttribute(const QString &key);
    bool isCategoryAttribute(const QString &key);
};

}

#endif /* MYIMAGEINFO_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
