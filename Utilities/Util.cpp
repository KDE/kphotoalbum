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

#include "Util.h"
#include "Logging.h"

#include <DB/CategoryCollection.h>
#include <DB/ImageDB.h>
#include <ImageManager/ImageDecoder.h>
#include <ImageManager/RawImageDecoder.h>
#include <Settings/SettingsData.h>

#include <KLocalizedString>
#include <KMessageBox>

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QMimeDatabase>
#include <QMimeType>
#include <QStandardPaths>
#include <QTextCodec>

extern "C" {
#include <unistd.h>
}

void Utilities::checkForBackupFile( const QString& fileName, const QString& message )
{
    QString backupName = QFileInfo( fileName ).absolutePath() + QString::fromLatin1("/.#") + QFileInfo( fileName ).fileName();
    QFileInfo backUpFile( backupName);
    QFileInfo indexFile( fileName );

    if ( !backUpFile.exists() || indexFile.lastModified() > backUpFile.lastModified() || backUpFile.size() == 0 )
        if ( !( backUpFile.exists() && !message.isNull() ) )
            return;

    int code;
    if ( message.isNull() )
        code = KMessageBox::questionYesNo( nullptr, i18n("Autosave file '%1' exists (size %3 KB) and is newer than '%2'. "
                "Should the autosave file be used?", backupName, fileName, backUpFile.size() >> 10 ),
                i18n("Found Autosave File") );
    else if ( backUpFile.size() > 0 )
        code = KMessageBox::warningYesNo( nullptr,i18n( "<p>Error: Cannot use current database file '%1':</p><p>%2</p>"
                "<p>Do you want to use autosave (%3 - size %4 KB) instead of exiting?</p>"
                "<p><small>(Manually verifying and copying the file might be a good idea)</small></p>", fileName, message, backupName, backUpFile.size() >> 10 ),
                i18n("Recover from Autosave?") );
    else {
        KMessageBox::error( nullptr, i18n( "<p>Error: %1</p><p>Also autosave file is empty, check manually "
                        "if numbered backup files exist and can be used to restore index.xml.</p>", message ) );
        exit(-1);
    }

    if ( code == KMessageBox::Yes ) {
        QFile in( backupName );
        if ( in.open( QIODevice::ReadOnly ) ) {
            QFile out( fileName );
            if (out.open( QIODevice::WriteOnly ) ) {
                char data[1024];
                int len;
                while ( (len = in.read( data, 1024 ) ) )
                    out.write( data, len );
            }
        }
    } else if ( !message.isNull() )
        exit(-1);
}

bool Utilities::copy( const QString& from, const QString& to )
{
    if ( QFileInfo(to).exists())
        QDir().remove(to);
    return QFile::copy(from,to);
}

bool Utilities::makeHardLink( const QString& from, const QString& to )
{
    if (link(from.toLocal8Bit().constData(), to.toLocal8Bit().constData()) != 0)
        return false;
    else
        return true;
}

bool Utilities::makeSymbolicLink( const QString& from, const QString& to )
{
    if (symlink(from.toLocal8Bit().constData(), to.toLocal8Bit().constData()) != 0)
        return false;
    else
        return true;
}

bool Utilities::canReadImage( const DB::FileName& fileName )
{
    bool fastMode = !Settings::SettingsData::instance()->ignoreFileExtension();
    QMimeDatabase::MatchMode mode = fastMode ? QMimeDatabase::MatchExtension : QMimeDatabase::MatchDefault;
    QMimeDatabase db;
    QMimeType mimeType = db.mimeTypeForFile( fileName.absolute(), mode );

    return QImageReader::supportedMimeTypes().contains( mimeType.name().toUtf8() )
            || ImageManager::ImageDecoder::mightDecode( fileName );
}


QString Utilities::locateDataFile(const QString& fileName)
{
    return QStandardPaths::locate(QStandardPaths::DataLocation, fileName);
}

QColor Utilities::contrastColor( const QColor& col )
{
    if ( col.red() < 127 && col.green() < 127 && col.blue() < 127 )
        return Qt::white;
    else
        return Qt::black;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
