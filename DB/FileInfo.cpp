/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
#include <qdatetime.h>
#include <qfileinfo.h>
#include "Utilities/Util.h"
#include <config-kpa-exiv2.h>
#ifdef HAVE_EXIV2
#  include "Exif/Info.h"
#endif
#include <kfilemetainfo.h>

using namespace DB;

FileInfo FileInfo::read( const QString& fileName, DB::ExifMode mode )
{
    return FileInfo( fileName, mode );
}

DB::FileInfo::FileInfo( const QString& fileName, DB::ExifMode mode )
    : _angle(0)
{
#ifdef HAVE_EXIV2
    parseEXIV2( fileName );
#else
    parseKFileMetaInfo( fileName );
#endif


    if ( updateDatFromFileTimeStamp(fileName,mode))
        _date = QFileInfo( fileName ).lastModified();
}

bool DB::FileInfo::updateDatFromFileTimeStamp(const QString& fileName, DB::ExifMode mode)
{
    // If the date is valid from EXIF reading, then we should not use the time stamp from the file.
    if ( _date.isValid() )
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

#ifdef HAVE_EXIV2
void DB::FileInfo::parseEXIV2( const QString& fileName )
{
    Exiv2::ExifData map = Exif::Info::instance()->metadata( fileName ).exif;

    // Date
    _date = fetchEXIV2Date( map, "Exif.Photo.DateTimeOriginal" );
    if ( !_date.isValid() ) {
        _date = fetchEXIV2Date( map, "Exif.Photo.DateTimeDigitized" );
        if ( !_date.isValid() )
            _date = fetchEXIV2Date( map, "Exif.Image.DateTime" );
    }

    // Angle
    if ( map.findKey( Exiv2::ExifKey( "Exif.Image.Orientation" ) ) != map.end() ) {
        const Exiv2::Exifdatum& datum = map["Exif.Image.Orientation"];

        int orientation =  datum.toLong();
        _angle = orientationToAngle( orientation );
    }

    // Description
    if( map.findKey( Exiv2::ExifKey( "Exif.Image.ImageDescription" ) ) != map.end() ) {
        const Exiv2::Exifdatum& datum = map["Exif.Image.ImageDescription"];
        _description = QString::fromLocal8Bit( datum.toString().c_str() );
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
#endif

void DB::FileInfo::parseKFileMetaInfo( const QString& fileName )
{
    QString tempFileName( fileName );
#ifdef REMOVED_FOR_SOME_REASON_ALSO_IN_KDE3
    if ( Util::isCRW( fileName ) ) {
      QString dirName = QFileInfo( fileName ).path();
      QString baseName = QFileInfo( fileName ).baseName();
      tempFileName = dirName + QString::fromLatin1("/") + baseName + QString::fromLatin1( ".thm" );
      QFileInfo tempFile (tempFileName);
      if ( !tempFile.exists() )
          tempFileName = dirName + QString::fromLatin1("/") + baseName + QString::fromLatin1( ".THM" );
    }
#endif

    KFileMetaInfo metainfo( tempFileName );
    if ( !metainfo.isValid() )
        return;

    // Date.
    if ( metainfo.keys().contains( QString::fromLatin1( "CreationDate" ) ) ) {
        QDate date = metainfo.item( QString::fromLatin1( "CreationDate" )).value().toDate();
        if ( date.isValid() ) {
            _date.setDate( date );

            if ( metainfo.keys().contains( QString::fromLatin1( "CreationTime" ) ) ) {
                QTime time = metainfo.item(QString::fromLatin1( "CreationTime" )).value().toTime();
                if ( time.isValid() )
                    _date.setTime( time );
            }
        }
    }

    // Angle
    if ( metainfo.keys().contains( QString::fromLatin1( "Orientation" ) ) )
        _angle = orientationToAngle( metainfo.item( QString::fromLatin1( "Orientation" ) ).value().toInt() );

    // Description
    if ( metainfo.keys().contains( QString::fromLatin1( "Comment" ) ) )
        _description = metainfo.item( QString::fromLatin1( "Comment" ) ).value().toString();
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

