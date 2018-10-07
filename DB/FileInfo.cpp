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
#include "FileInfo.h"

#include <Exif/Info.h>
#include <QRegularExpression>
#include <Settings/SettingsData.h>
#include <Utilities/Util.h>

#include <QDateTime>
#include <QFileInfo>

using namespace DB;

FileInfo FileInfo::read( const DB::FileName& fileName, DB::ExifMode mode )
{
    return FileInfo( fileName, mode );
}

DB::FileInfo::FileInfo( const DB::FileName& fileName, DB::ExifMode mode )
    : m_angle(0),
      m_fileName(fileName)
{
    parseEXIV2( fileName );


    if ( updateDataFromFileTimeStamp(fileName,mode))
        m_date = QFileInfo( fileName.absolute() ).lastModified();
}

Exiv2::ExifData& DB::FileInfo::getExifData()
{
    return m_exifMap;
}

const DB::FileName& DB::FileInfo::getFileName() const
{
    return m_fileName;
}

bool DB::FileInfo::updateDataFromFileTimeStamp(const DB::FileName& fileName, DB::ExifMode mode)
{
    // If the date is valid from Exif reading, then we should not use the time stamp from the file.
    if ( m_date.isValid() )
        return false;

    // If we are not setting date, then we should of course not set the date
    if ( (mode & EXIFMODE_DATE) == 0 )
        return false;

    // If we are we already have specifies that we want to sent the date (from the ReReadExif dialog), then we of course should.
    if ( (mode & EXIFMODE_USE_IMAGE_DATE_IF_INVALID_EXIF_DATE ) != 0)
        return true;

    // Always trust for videos (this is a way to say that we should not trust for scaned in images - which makes no sense for videos)
    if ( Utilities::isVideo(fileName) )
        return true;

    // Finally use the info from the settings dialog
    return Settings::SettingsData::instance()->trustTimeStamps();

}

void DB::FileInfo::parseEXIV2( const DB::FileName& fileName )
{
    m_exifMap = Exif::Info::instance()->metadata( fileName ).exif;

    // Date
    m_date = fetchEXIV2Date( m_exifMap, "Exif.Photo.DateTimeOriginal" );
    if ( !m_date.isValid() ) {
        m_date = fetchEXIV2Date( m_exifMap, "Exif.Photo.DateTimeDigitized" );
        if ( !m_date.isValid() )
            m_date = fetchEXIV2Date( m_exifMap, "Exif.Image.DateTime" );
    }

    // Angle
    if ( m_exifMap.findKey( Exiv2::ExifKey( "Exif.Image.Orientation" ) ) != m_exifMap.end() ) {
        const Exiv2::Exifdatum& datum = m_exifMap["Exif.Image.Orientation"];

        int orientation = 0;
        if (datum.count() > 0)
            orientation =  datum.toLong();
        m_angle = orientationToAngle( orientation );
    }

    // Description
    if( m_exifMap.findKey( Exiv2::ExifKey( "Exif.Image.ImageDescription" ) ) != m_exifMap.end() ) {
        const Exiv2::Exifdatum& datum = m_exifMap["Exif.Image.ImageDescription"];
        m_description = QString::fromLocal8Bit( datum.toString().c_str() ).trimmed();
        // some cameras seem to add control characters. Remove them:
        m_description.remove(QRegularExpression(QString::fromLatin1("\\p{Cc}")));
    }
}

QDateTime FileInfo::fetchEXIV2Date( Exiv2::ExifData& map, const char* key )
{
    try
    {
        if ( map.findKey( Exiv2::ExifKey( key ) ) != map.end() ) {
            const Exiv2::Exifdatum& datum = map[key ];
            return QDateTime::fromString( QString::fromLatin1(datum.toString().c_str()), Qt::ISODate );
        }
    }
    catch (...)
    {
    }

    return QDateTime();
}

int DB::FileInfo::orientationToAngle( int orientation )
{
    if ( orientation == 1 || orientation == 2 )
        return 0;
    else if ( orientation == 3 || orientation == 4 )
        return 180;
    else if ( orientation == 5 || orientation == 8 )
        return 270;
    else if ( orientation == 6 || orientation == 7 )
        return 90;

    return 0;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
