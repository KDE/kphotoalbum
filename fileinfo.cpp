#include "fileinfo.h"
#include <qdatetime.h>
#include <qfileinfo.h>
#include "util.h"
#include "set.h"
#ifdef HASEXIV2
#  include "Exif/Info.h"
#endif

FileInfo FileInfo::read( const QString& fileName )
{
    FileInfo fi;
    fi._fullPath = fileName;

#ifdef HASEXIV2
    fi._map = Exif::Info::instance()->exifData( fileName );
#endif

    return fi;
}

QTime FileInfo::time()
{
#ifdef HASEXIV2
    if ( _map.findKey( Exiv2::ExifKey( "Exif.Image.DateTime" ) ) != _map.end() ) {
        const Exiv2::Exifdatum& datum = _map["Exif.Image.DateTime"];
        QTime time = QDateTime::fromString( QString::fromLatin1(datum.toString().c_str()), Qt::ISODate ).time();
        if ( time.isValid() )
            return time;
    }
#endif
    return QFileInfo( _fullPath ).lastModified().time();
}

QDate FileInfo::date()
{
#ifdef HASEXIV2
    if ( _map.findKey( Exiv2::ExifKey( "Exif.Image.DateTime" ) ) != _map.end() ) {
        const Exiv2::Exifdatum& datum = _map["Exif.Image.DateTime"];
        QDate date = QDateTime::fromString( QString::fromLatin1(datum.toString().c_str()), Qt::ISODate ).date();
        if ( date.isValid() )
            return date;
    }
#endif
    return QFileInfo( _fullPath ).lastModified().date();
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
