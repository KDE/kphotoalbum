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
#ifndef EXIF_INFO_H
#define EXIF_INFO_H
#include <Utilities/StringSet.h>

#include <exiv2/exif.hpp>
#include <exiv2/iptc.hpp>
#include <qmap.h>
#include <qstringlist.h>

namespace DB
{
class FileName;
}

namespace Exif
{

using Utilities::StringSet;

struct Metadata {
    Exiv2::ExifData exif;
    Exiv2::IptcData iptc;
    std::string comment;
};

class Info
{
public:
    Info();
    static Info *instance();
    QMap<QString, QStringList> info(const DB::FileName &fileName, StringSet wantedKeys, bool returnFullExifName, const QString &charset);
    QMap<QString, QStringList> infoForViewer(const DB::FileName &fileName, const QString &charset);
    QMap<QString, QStringList> infoForDialog(const DB::FileName &fileName, const QString &charset);
    StringSet availableKeys();
    StringSet standardKeys();
    void writeInfoToFile(const DB::FileName &srcName, const QString &destName);
    Metadata metadata(const DB::FileName &fileName);

protected:
    DB::FileName exifInfoFile(const DB::FileName &fileName);

private:
    static Info *s_instance;
    StringSet m_keys;
};

}

#endif /* EXIF_INFO_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
