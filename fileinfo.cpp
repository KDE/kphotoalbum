#include "fileinfo.h"
#include <kfilemetainfo.h>
#include <qdatetime.h>
#include <qfileinfo.h>
#include "util.h"

FileInfo FileInfo::read( const QString& fileName )
{
    FileInfo fi;
    fi._fullPath = fileName;
    QString tempFileName( fileName );
    if ( Util::isCRW( fileName ) ) {
      QString dirName = QFileInfo( fileName ).dirPath();
      QString baseName = QFileInfo( fileName ).baseName();
      tempFileName = dirName + QString::fromLatin1("/") + baseName + QString::fromLatin1( ".thm" );
      QFileInfo tempFile (tempFileName);
      if ( !tempFile.exists() )
          tempFileName = dirName + QString::fromLatin1("/") + baseName + QString::fromLatin1( ".THM" );
    }

    KFileMetaInfo metainfo( tempFileName );
    if ( metainfo.isEmpty() )
        return fi;

    QStringList keys = metainfo.supportedKeys();
    for( QStringList::Iterator it = keys.begin(); it != keys.end(); ++it ) {
        KFileMetaInfoItem item = metainfo.item( *it );
        if (item.type() != QVariant::Invalid) {
            fi._map.insert( *it, item.value() );
        }
    }
    return fi;
}

bool FileInfo::isEmpty() const
{
    return _map.count() == 0;
}

QTime FileInfo::time( bool* foundTimeInExif ) const
{
    if ( _map.contains( QString::fromLatin1( "CreationTime" ) ) ) {
        QTime time = _map[QString::fromLatin1( "CreationTime" )].toTime();
        if ( time.isValid() ) {
            if ( foundTimeInExif )
                *foundTimeInExif = true;
            return time;
        }
    }

    if ( foundTimeInExif )
        *foundTimeInExif = false;

    return QFileInfo( _fullPath ).lastModified().time();
}

QDate FileInfo::date( bool* foundDateInExif ) const
{
    if ( _map.contains( QString::fromLatin1( "CreationDate" ) ) ) {
        QDate date = _map[QString::fromLatin1( "CreationDate" )].toDate();
        if ( date.isValid() ) {
            if ( foundDateInExif )
                *foundDateInExif = true;
            return date;
        }
    }

    if ( foundDateInExif )
        *foundDateInExif = false;

    return QFileInfo( _fullPath ).lastModified().date();
}

int FileInfo::angle( bool* found ) const
{
    if ( !_map.contains(QString::fromLatin1( "Orientation" )) ) {
        if ( found )
            *found = false;
        return 0;
    }

    if ( found )
        *found = true;

    int orientation =  _map[QString::fromLatin1( "Orientation" )].toInt();
    if ( orientation == 1 || orientation == 2 )
        return 0;
    else if ( orientation == 3 || orientation == 4 )
        return 180;
    else if ( orientation == 5 || orientation == 8 )
        return 270;
    else if ( orientation == 6 || orientation == 7 )
        return 90;
    else {
        if ( found )
            *found = false;
        return 0;
    }
}

QString FileInfo::description( bool* found) const
{
    if ( !_map.contains(QString::fromLatin1( "Comment" )) ) {
        if ( found )
            found = false;
    }
    return _map[QString::fromLatin1( "Comment" )].toString();
}
