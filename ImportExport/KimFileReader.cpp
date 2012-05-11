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
#include "KimFileReader.h"
#include <QFileInfo>
#include <klocale.h>
#include <kmessagebox.h>
#include <kzip.h>
#include "Utilities/Util.h"

ImportExport::KimFileReader::KimFileReader()
    :_zip(0)
{
}

bool ImportExport::KimFileReader::open( const QString& fileName )
{
    m_fileName = fileName;
    _zip = new KZip( fileName );
    if ( !_zip->open( QIODevice::ReadOnly ) ) {
        KMessageBox::error( 0, i18n("Unable to open '%1' for reading.", fileName ), i18n("Error Importing Data") );
        delete _zip;
	_zip = 0;
        return false;
    }

    _dir = _zip->directory();
    if ( _dir == 0 ) {
        KMessageBox::error( 0, i18n( "Error reading directory contents of file %1; it is likely that the file is broken." , fileName ) );
        delete _zip;
	_zip = 0;
	return false;
    }


    return true;
}

QByteArray ImportExport::KimFileReader::indexXML()
{
    const KArchiveEntry* indexxml = _dir->entry( QString::fromLatin1( "index.xml" ) );
    if ( indexxml == 0 || ! indexxml->isFile() ) {
        KMessageBox::error( 0, i18n( "Error reading index.xml file from %1; it is likely that the file is broken." , m_fileName ) );
        return QByteArray();
    }

    const KArchiveFile* file = static_cast<const KArchiveFile*>( indexxml );
    return file->data();
}

ImportExport::KimFileReader::~KimFileReader()
{
    delete _zip;
}

QPixmap ImportExport::KimFileReader::loadThumbnail( QString fileName )
{
    const KArchiveEntry* thumbnails = _dir->entry( QString::fromLatin1( "Thumbnails" ) );
    if( !thumbnails )
        return QPixmap();

    if ( !thumbnails->isDirectory() ) {
        KMessageBox::error( 0, i18n("Thumbnail item in export file was not a directory, this indicates that the file is broken.") );
        return QPixmap();
    }

    const KArchiveDirectory* thumbnailDir = static_cast<const KArchiveDirectory*>( thumbnails );

    const QString ext = Utilities::isVideo( DB::FileName::fromRelativePath(fileName) ) ? QString::fromLatin1( "jpg" ) : QFileInfo( fileName ).completeSuffix();
    fileName = QString::fromLatin1("%1.%2").arg( Utilities::stripEndingForwardSlash( QFileInfo( fileName ).baseName() ) ).arg(ext);
    const KArchiveEntry* fileEntry = thumbnailDir->entry( fileName );
    if ( fileEntry == 0 || !fileEntry->isFile() ) {
        KMessageBox::error( 0, i18n("No thumbnail existed in export file for %1", fileName ) );
        return QPixmap();
    }

    const KArchiveFile* file = static_cast<const KArchiveFile*>( fileEntry );
    QByteArray data = file->data();
    QPixmap pixmap;
    pixmap.loadFromData( data );
    return pixmap;
}

QByteArray ImportExport::KimFileReader::loadImage( const QString& fileName )
{
    const KArchiveEntry* images = _dir->entry( QString::fromLatin1( "Images" ) );
    if ( !images ) {
        KMessageBox::error( 0, i18n("export file did not contain a Images subdirectory, this indicates that the file is broken") );
        return QByteArray();
    }

    if ( !images->isDirectory() ) {
        KMessageBox::error( 0, i18n("Images item in export file was not a directory, this indicates that the file is broken") );
        return QByteArray();
    }

    const KArchiveDirectory* imagesDir = static_cast<const KArchiveDirectory*>( images );

    const KArchiveEntry* fileEntry = imagesDir->entry( fileName );
    if ( fileEntry == 0 || !fileEntry->isFile() ) {
        KMessageBox::error( 0, i18n("No image existed in export file for %1", fileName ) );
        return QByteArray();
    }

    const KArchiveFile* file = static_cast<const KArchiveFile*>( fileEntry );
    QByteArray data = file->data();
    return data;
}
