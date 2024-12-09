// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef EXIF_INFO_H
#define EXIF_INFO_H
#include <kpabase/StringSet.h>

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
    Metadata metadata(const DB::FileName &fileName);

protected:
    DB::FileName exifInfoFile(const DB::FileName &fileName);

private:
    static Info *s_instance;
    StringSet m_keys;
};

/**
 * @brief Writes the exif information from one file to another.
 * The given description is overwrites the exif description field of the destination file.
 * @param srcName the image file in the database
 * @param destName the destination image file
 * @param imageDescription the image description text (usually DB::ImageInfo::description())
 */
void writeExifInfoToFile(const DB::FileName &srcName, const QString &destName, const QString &imageDescription);

}

#endif /* EXIF_INFO_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
