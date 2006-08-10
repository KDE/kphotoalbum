#include "FileInfo.h"
#include <qdatetime.h>
#include <qfileinfo.h>
#include "Utilities/Util.h"
#ifdef HASEXIV2
#  include "Exif/Info.h"
#endif

using namespace DB;

FileInfo FileInfo::read( const QString& fileName )
{
    FileInfo fi;
    fi._fullPath = fileName;

#ifdef HASEXIV2
    fi._map = Exif::Info::instance()->exifData( fileName );
#endif

    return fi;
}

int FileInfo::angle()
{
#ifdef HASEXIV2
    if ( _map.findKey( Exiv2::ExifKey( "Exif.Image.Orientation" ) ) != _map.end() ) {
        const Exiv2::Exifdatum& datum = _map["Exif.Image.Orientation"];

        int orientation =  datum.toLong();
        if ( orientation == 1 || orientation == 2 )
            return 0;
        else if ( orientation == 3 || orientation == 4 )
            return 180;
        else if ( orientation == 5 || orientation == 8 )
            return 270;
        else if ( orientation == 6 || orientation == 7 )
            return 90;
    }
#endif
    return 0;
}

QString FileInfo::description()
{
#ifdef HASEXIV2
    if( _map.findKey( Exiv2::ExifKey( "Exif.Image.ImageDescription" ) ) != _map.end() ) {
        const Exiv2::Exifdatum& datum = _map["Exif.Image.ImageDescription"];
        return QString::fromLatin1( datum.toString().c_str() );
    }
#endif
    return QString::null;
}

QDateTime FileInfo::dateTime()
{
#ifdef HASEXIV2
    QDateTime date = fetchDate( "Exif.Photo.DateTimeOriginal" );
    if ( date.isValid() )
        return date;

    date = fetchDate( "Exif.Photo.DateTimeDigitized" );
    if ( date.isValid() )
        return date;

    date = fetchDate( "Exif.Image.DateTime" );
    if ( date.isValid() )
        return date;

#endif
    if ( Settings::SettingsData::instance()->trustTimeStamps() )
        return QFileInfo( _fullPath ).lastModified();
    else
        return QDateTime();
}

QDateTime FileInfo::fetchDate( const char* key )
{
#ifdef HASEXIV2
    try
    {
        if ( _map.findKey( Exiv2::ExifKey( key ) ) != _map.end() ) {
            const Exiv2::Exifdatum& datum = _map[key ];
            return QDateTime::fromString( QString::fromLatin1(datum.toString().c_str()), Qt::ISODate );
        }
    }
    catch (...)
    {
    }
#endif
    return QDateTime();
}
