// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FILEINFO_H
#define FILEINFO_H

#include "ExifMode.h"

#include <kpaexif/Info.h>

#include <Utilities/FastDateTime.h>
#include <qstring.h>

namespace DB
{
class FileName;

class FileInfo
{
public:
    static FileInfo read(const DB::FileName &fileName, DB::ExifMode mode);
    Utilities::FastDateTime dateTime() { return m_date; }
    int angle() { return m_angle; }
    QString description() { return m_description; }
    Exiv2::ExifData &getExifData();
    const DB::FileName &getFileName() const;

protected:
    void parseEXIV2(const DB::FileName &fileName);
    Utilities::FastDateTime fetchEXIV2Date(Exiv2::ExifData &map, const char *key);

    int orientationToAngle(int orientation);

private:
    FileInfo(const DB::FileName &fileName, DB::ExifMode mode);
    bool updateDataFromFileTimeStamp(const DB::FileName &fileName, DB::ExifMode mode);
    Utilities::FastDateTime m_date;
    int m_angle;
    QString m_description;
    Exiv2::ExifData m_exifMap;
    const DB::FileName &m_fileName;
};

}

#endif /* FILEINFO_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
