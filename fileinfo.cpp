#include "fileinfo.h"
#include <qdatetime.h>
#include <qfileinfo.h>
#include "util.h"
#include <config.h>
#include "set.h"
#ifdef HASEXIV2
#  include "Exif/Info.h"
#endif

FileInfo FileInfo::read( const QString& fileName )
{
    FileInfo fi;
    fi._fullPath = fileName;
    QString tempFileName( fileName );

#if 0
    // PENDING(blackie) What do we do about this?
    if ( Util::isCRW( fileName ) ) {
      QString dirName = QFileInfo( fileName ).dirPath();
      QString baseName = QFileInfo( fileName ).baseName();
      tempFileName = dirName + QString::fromLatin1("/") + baseName + QString::fromLatin1( ".thm" );
      QFileInfo tempFile (tempFileName);
      if ( !tempFile.exists() )
          tempFileName = dirName + QString::fromLatin1("/") + baseName + QString::fromLatin1( ".THM" );
    }
#endif

#ifdef HASEXIV2
    Set<QString> wantedKeys;
    wantedKeys.insert( QString::fromLatin1( "Exif.Image.ImageDescription" ) );
    wantedKeys.insert( QString::fromLatin1( "Exif.Image.Orientation" ) );
    wantedKeys.insert( QString::fromLatin1( "Exif.Image.DateTime" ) );
    fi._map = Exif::Info::instance()->info( tempFileName, wantedKeys, false );
#endif

    return fi;
}

QTime FileInfo::time() const
{
    if ( _map.contains( QString::fromLatin1( "DateTime" ) ) ) {
        QTime time = QDateTime::fromString( _map[QString::fromLatin1( "DateTime" )], Qt::ISODate ).time();
        if ( time.isValid() )
            return time;
    }

    return QFileInfo( _fullPath ).lastModified().time();
}

QDate FileInfo::date() const
{
    if ( _map.contains( QString::fromLatin1( "DateTime" ) ) ) {
        QDate date = QDateTime::fromString( _map[QString::fromLatin1( "DateTime" )], Qt::ISODate ).date();
        if ( date.isValid() )
            return date;
    }

    return QFileInfo( _fullPath ).lastModified().date();
}

int FileInfo::angle() const
{
    if ( !_map.contains(QString::fromLatin1( "Orientation" )) )
        return 0;

    int orientation =  _map[QString::fromLatin1( "Orientation" )].toInt();
    if ( orientation == 1 || orientation == 2 )
        return 0;
    else if ( orientation == 3 || orientation == 4 )
        return 180;
    else if ( orientation == 5 || orientation == 8 )
        return 270;
    else if ( orientation == 6 || orientation == 7 )
        return 90;
    else
        return 0;
}

QString FileInfo::description() const
{
    if ( _map.contains(QString::fromLatin1( "ImageDescription" )) )
        return _map[QString::fromLatin1( "ImageDescription" )];
    return QString::null;
}
