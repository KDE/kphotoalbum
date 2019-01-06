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
#include <KLocalizedString>
#include <kmessagebox.h>
#include <kzip.h>
#include <Utilities/FileNameUtil.h>
#include <Utilities/VideoUtil.h>

ImportExport::KimFileReader::KimFileReader()
    :m_zip(nullptr)
{
}

bool ImportExport::KimFileReader::open( const QString& fileName )
{
    m_fileName = fileName;
    m_zip = new KZip( fileName );
    if ( !m_zip->open( QIODevice::ReadOnly ) ) {
        KMessageBox::error( nullptr, i18n("Unable to open '%1' for reading.", fileName ), i18n("Error Importing Data") );
        delete m_zip;
    m_zip = nullptr;
        return false;
    }

    m_dir = m_zip->directory();
    if ( m_dir == nullptr ) {
        KMessageBox::error( nullptr, i18n( "Error reading directory contents of file %1; it is likely that the file is broken." , fileName ) );
        delete m_zip;
    m_zip = nullptr;
    return false;
    }


    return true;
}

QByteArray ImportExport::KimFileReader::indexXML()
{
    const KArchiveEntry* indexxml = m_dir->entry( QString::fromLatin1( "index.xml" ) );
    if ( indexxml == nullptr || ! indexxml->isFile() ) {
        KMessageBox::error( nullptr, i18n( "Error reading index.xml file from %1; it is likely that the file is broken." , m_fileName ) );
        return QByteArray();
    }

    const KArchiveFile* file = static_cast<const KArchiveFile*>( indexxml );
    return file->data();
}

ImportExport::KimFileReader::~KimFileReader()
{
    delete m_zip;
}

QPixmap ImportExport::KimFileReader::loadThumbnail( QString fileName )
{
    const KArchiveEntry* thumbnails = m_dir->entry( QString::fromLatin1( "Thumbnails" ) );
    if( !thumbnails )
        return QPixmap();

    if ( !thumbnails->isDirectory() ) {
        KMessageBox::error( nullptr, i18n("Thumbnail item in export file was not a directory, this indicates that the file is broken.") );
        return QPixmap();
    }

    const KArchiveDirectory* thumbnailDir = static_cast<const KArchiveDirectory*>( thumbnails );

    const QString ext = Utilities::isVideo( DB::FileName::fromRelativePath(fileName) ) ? QString::fromLatin1( "jpg" ) : QFileInfo( fileName ).completeSuffix();
    fileName = QString::fromLatin1("%1.%2").arg( Utilities::stripEndingForwardSlash( QFileInfo( fileName ).baseName() ) ).arg(ext);
    const KArchiveEntry* fileEntry = thumbnailDir->entry( fileName );
    if ( fileEntry == nullptr || !fileEntry->isFile() ) {
        KMessageBox::error( nullptr, i18n("No thumbnail existed in export file for %1", fileName ) );
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
    const KArchiveEntry* images = m_dir->entry( QString::fromLatin1( "Images" ) );
    if ( !images ) {
        KMessageBox::error( nullptr, i18n("export file did not contain a Images subdirectory, this indicates that the file is broken") );
        return QByteArray();
    }

    if ( !images->isDirectory() ) {
        KMessageBox::error( nullptr, i18n("Images item in export file was not a directory, this indicates that the file is broken") );
        return QByteArray();
    }

    const KArchiveDirectory* imagesDir = static_cast<const KArchiveDirectory*>( images );

    const KArchiveEntry* fileEntry = imagesDir->entry( fileName );
    if ( fileEntry == nullptr || !fileEntry->isFile() ) {
        KMessageBox::error( nullptr, i18n("No image existed in export file for %1", fileName ) );
        return QByteArray();
    }

    const KArchiveFile* file = static_cast<const KArchiveFile*>( fileEntry );
    QByteArray data = file->data();
    return data;
}
// vi:expandtab:tabstop=4 shiftwidth=4:
