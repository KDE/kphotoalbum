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
#include <Exif/Info.h>
#include <ImageManager/ImageDecoder.h>
#include <ImageManager/RawImageDecoder.h>
#include <MainWindow/Window.h>
#include <Settings/SettingsData.h>

#include <KLocalizedString>
#include <KMessageBox>

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QList>
#include <QMimeDatabase>
#include <QMimeType>
#include <QStandardPaths>
#include <QTextCodec>
#include <QUrl>

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

QString Utilities::readFile( const QString& fileName )
{
    if ( fileName.isEmpty() ) {
        KMessageBox::error( nullptr, i18n("<p>No file name given!</p>") );
        return QString();
    }

    QFile file( fileName );
    if ( !file.open( QIODevice::ReadOnly ) ) {
        //KMessageBox::error( nullptr, i18n("Could not open file %1").arg( fileName ) );
        return QString();
    }

    QTextStream stream( &file );
    QString content = stream.readAll();
    file.close();

    return content;
}

namespace Utilities
{
QString normalizedFileName( const QString& fileName )
{
    return QFileInfo(fileName).absoluteFilePath();
}

QString dereferenceSymLinks( const QString& fileName )
{
    QFileInfo fi(fileName);
    int rounds = 256;
    while (fi.isSymLink() && --rounds > 0)
        fi = QFileInfo(fi.readLink());
    if (rounds == 0)
        return QString();
    return fi.filePath();
}
}

QString Utilities::stripEndingForwardSlash( const QString& fileName )
{
    static QString slash = QString::fromLatin1("/");
    if ( fileName.endsWith( slash ) )
        return fileName.left( fileName.length()-1);
    else
        return fileName;
}

QString Utilities::relativeFolderName( const QString& fileName)
{
    int index= fileName.lastIndexOf( QChar::fromLatin1('/'), -1);
    if (index == -1)
        return QString();
    else
        return fileName.left( index );
}

QString Utilities::absoluteImageFileName( const QString& relativeName )
{
    return stripEndingForwardSlash( Settings::SettingsData::instance()->imageDirectory() ) + QString::fromLatin1( "/" ) + relativeName;
}

QString Utilities::imageFileNameToAbsolute( const QString& fileName )
{
    if ( fileName.startsWith( Settings::SettingsData::instance()->imageDirectory() ) )
        return fileName;
    else if ( fileName.startsWith( QString::fromLatin1("file://") ) )
        return imageFileNameToAbsolute( fileName.mid( 7 ) ); // 7 == length("file://")
    else if ( fileName.startsWith( QString::fromLatin1("/") ) )
        return QString(); // Not within our image root
    else
        return absoluteImageFileName( fileName );
}


const QSet<QString>& Utilities::supportedVideoExtensions()
{
    static QSet<QString> videoExtensions;
    if ( videoExtensions.empty() ) {
        videoExtensions.insert( QString::fromLatin1( "3gp" ) );
        videoExtensions.insert( QString::fromLatin1( "avi" ) );
        videoExtensions.insert( QString::fromLatin1( "mp4" ) );
        videoExtensions.insert( QString::fromLatin1( "m4v" ) );
        videoExtensions.insert( QString::fromLatin1( "mpeg" ) );
        videoExtensions.insert( QString::fromLatin1( "mpg" ) );
        videoExtensions.insert( QString::fromLatin1( "qt" ) );
        videoExtensions.insert( QString::fromLatin1( "mov" ) );
        videoExtensions.insert( QString::fromLatin1( "moov" ) );
        videoExtensions.insert( QString::fromLatin1( "qtvr" ) );
        videoExtensions.insert( QString::fromLatin1( "rv" ) );
        videoExtensions.insert( QString::fromLatin1( "3g2" ) );
        videoExtensions.insert( QString::fromLatin1( "fli" ) );
        videoExtensions.insert( QString::fromLatin1( "flc" ) );
        videoExtensions.insert( QString::fromLatin1( "mkv" ) );
        videoExtensions.insert( QString::fromLatin1( "mng" ) );
        videoExtensions.insert( QString::fromLatin1( "asf" ) );
        videoExtensions.insert( QString::fromLatin1( "asx" ) );
        videoExtensions.insert( QString::fromLatin1( "wmp" ) );
        videoExtensions.insert( QString::fromLatin1( "wmv" ) );
        videoExtensions.insert( QString::fromLatin1( "ogm" ) );
        videoExtensions.insert( QString::fromLatin1( "rm" ) );
        videoExtensions.insert( QString::fromLatin1( "flv" ) );
        videoExtensions.insert( QString::fromLatin1( "webm" ) );
        videoExtensions.insert( QString::fromLatin1( "mts" ) );
        videoExtensions.insert( QString::fromLatin1( "ogg" ) );
        videoExtensions.insert( QString::fromLatin1( "ogv" ) );
        videoExtensions.insert( QString::fromLatin1( "m2ts" ) );
    }
    return videoExtensions;
}
bool Utilities::isVideo( const DB::FileName& fileName )
{
    QFileInfo fi( fileName.relative() );
    QString ext = fi.suffix().toLower();
    return supportedVideoExtensions().contains( ext );
}

bool Utilities::isRAW( const DB::FileName& fileName )
{
    return ImageManager::RAWImageDecoder::isRAW( fileName );
}


QImage Utilities::scaleImage(const QImage &image, int w, int h, Qt::AspectRatioMode mode )
{
    return image.scaled( w, h, mode, Settings::SettingsData::instance()->smoothScale() ? Qt::SmoothTransformation : Qt::FastTransformation );
}

QImage Utilities::scaleImage(const QImage &image, const QSize& s, Qt::AspectRatioMode mode )
{
    return scaleImage( image, s.width(), s.height(), mode );
}

QString Utilities::cStringWithEncoding( const char *c_str, const QString& charset )
{
    QTextCodec* codec = QTextCodec::codecForName( charset.toLatin1() );
    if (!codec)
        codec = QTextCodec::codecForLocale();
    return codec->toUnicode( c_str );
}

QColor Utilities::contrastColor( const QColor& col )
{
    if ( col.red() < 127 && col.green() < 127 && col.blue() < 127 )
        return Qt::white;
    else
        return Qt::black;
}

void Utilities::saveImage( const DB::FileName& fileName, const QImage& image, const char* format )
{
    const QFileInfo info(fileName.absolute());
    QDir().mkpath(info.path());
     const bool ok = image.save(fileName.absolute(),format);
    Q_ASSERT(ok); Q_UNUSED(ok);
}
// vi:expandtab:tabstop=4 shiftwidth=4:
