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

#ifndef FILEINFO_H
#define FILEINFO_H

#include <qstring.h>
#include <qdatetime.h>
#include "Exif/Info.h"

#include "ExifMode.h"

namespace DB
{
class FileName;

class FileInfo
{
public:
    static FileInfo read( const DB::FileName& fileName, DB::ExifMode mode );
    QDateTime dateTime() { return m_date; }
    int angle() { return m_angle; }
    QString description() {return m_description; }
    Exiv2::ExifData& getExifData();
    const DB::FileName& getFileName() const;

protected:
    void parseEXIV2( const DB::FileName& fileName );
    QDateTime fetchEXIV2Date( Exiv2::ExifData& map, const char* key );

    int orientationToAngle( int orientation );

private:
    FileInfo( const DB::FileName& fileName, DB::ExifMode mode );
    bool updateDataFromFileTimeStamp( const DB::FileName& fileName, DB::ExifMode mode);
    QDateTime m_date;
    int m_angle;
    QString m_description;
    Exiv2::ExifData m_exifMap;
    const DB::FileName& m_fileName;
};

}

#endif /* FILEINFO_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
