/* Copyright (C) 2003-2019 The KPhotoAlbum Development Team

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
#include "NumberedBackup.h"
#include "Settings/SettingsData.h"
#include <kzip.h>
#include <kmessagebox.h>
#include <KLocalizedString>
#include <qregexp.h>
#include <qdir.h>
#include "Utilities/Util.h"

void XMLDB::NumberedBackup::makeNumberedBackup()
{
    deleteOldBackupFiles();

    int max = getMaxId();
    QString fileName;
    fileName.sprintf( "index.xml~%04d~", max+1 );

    if ( !QFileInfo( QString::fromLatin1( "%1/index.xml" ).arg( Settings::SettingsData::instance()->imageDirectory() ) ).exists() )
        return;

    if ( Settings::SettingsData::instance()->compressBackup() ) {
        QString fileNameWithExt = fileName + QString::fromLatin1( ".zip" );

        QString fileAndDir = QString::fromLatin1( "%1/%2" ).arg(Settings::SettingsData::instance()->imageDirectory() ).arg(fileNameWithExt);
        KZip zip( fileAndDir );
        if ( ! zip.open( QIODevice::WriteOnly ) ) {
            KMessageBox::error( nullptr, i18n("Error creating zip file %1",fileAndDir) );
            return;
        }

        if ( !zip.addLocalFile( QString::fromLatin1( "%1/index.xml" ).arg( Settings::SettingsData::instance()->imageDirectory() ), fileName ) )
        {
            KMessageBox::error( nullptr, i18n("Error writing file %1 to zip file %2", fileName, fileAndDir) );
        }
        zip.close();
    }
    else {
        Utilities::copyOrOverwrite( QString::fromLatin1( "%1/index.xml" ).arg( Settings::SettingsData::instance()->imageDirectory() ),
                    QString::fromLatin1( "%1/%2" ).arg( Settings::SettingsData::instance()->imageDirectory() ).arg( fileName ) );
    }
}


int XMLDB::NumberedBackup::getMaxId() const
{
    QStringList files = backupFiles();
    int max = 0;
    for( QStringList::ConstIterator fileIt = files.constBegin(); fileIt != files.constEnd(); ++fileIt ) {
        bool OK;
        max = qMax( max, idForFile( *fileIt, OK ) );
    }
    return max;
}

QStringList XMLDB::NumberedBackup::backupFiles() const
{
    QDir dir( Settings::SettingsData::instance()->imageDirectory() );
    return dir.entryList( QStringList() << QString::fromLatin1( "index.xml~*~*" ), QDir::Files );
}

int XMLDB::NumberedBackup::idForFile( const QString& fileName, bool& OK ) const
{
    QRegExp reg( QString::fromLatin1( "index\\.xml~([0-9]+)~(.zip)?" ) );
    if ( reg.exactMatch( fileName ) ) {
        OK = true;
        return reg.cap(1).toInt();
    }
    else {
        OK = false;
        return -1;
    }
}


void XMLDB::NumberedBackup::deleteOldBackupFiles()
{
    int maxId = getMaxId();
    int maxBackupFiles = Settings::SettingsData::instance()->backupCount();
    if ( maxBackupFiles == -1 )
        return;

    QStringList files = backupFiles();

    for( QStringList::ConstIterator fileIt = files.constBegin(); fileIt != files.constEnd(); ++fileIt ) {
        bool OK;
        int num = idForFile( *fileIt, OK );
        if ( OK && num <= maxId+1 - maxBackupFiles ) {
            (QDir( Settings::SettingsData::instance()->imageDirectory() )).remove( *fileIt );
        }

    }
}
// vi:expandtab:tabstop=4 shiftwidth=4:
