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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "NewImageFinder.h"
#include "DB/ImageDB.h"
#include <qfileinfo.h>
#include "Settings/SettingsData.h"
#include "Browser/BrowserWidget.h"
#include <qdir.h>
#include "Utilities/Util.h"
#include <qprogressdialog.h>
#include <klocale.h>
#include <qapplication.h>
#include <qeventloop.h>
#include <kmessagebox.h>
#include <kmdcodec.h>
#include <config.h>
#ifdef HASEXIV2
#  include "Exif/Database.h"
#endif

using namespace DB;

bool NewImageFinder::findImages()
{
    // Load the information from the XML file.
    QDict<void> loadedFiles( 6301 /* a large prime */ );

    QStringList images = DB::ImageDB::instance()->images();
    for( QStringList::ConstIterator it = images.begin(); it != images.end(); ++it ) {
        loadedFiles.insert( *it, (void*)0x1 /* void pointer to nothing I never need the value,
                                               just its existsance, must be != 0x0 though.*/ );
    }

    _pendingLoad.clear();
    searchForNewFiles( loadedFiles, Settings::SettingsData::instance()->imageDirectory() );
    loadExtraFiles();

    // To avoid deciding if the new images are shown in a given thumbnail view or in a given search
    // we rather just go to home.
    Browser::BrowserWidget::instance()->home();
    return (!_pendingLoad.isEmpty()); // returns if new images was found.
}

void NewImageFinder::searchForNewFiles( const QDict<void>& loadedFiles, QString directory )
{
    if ( directory.endsWith( QString::fromLatin1("/") ) )
        directory = directory.mid( 0, directory.length()-1 );

    QString imageDir = Settings::SettingsData::instance()->imageDirectory();
    if ( imageDir.endsWith( QString::fromLatin1("/") ) )
        imageDir = imageDir.mid( 0, imageDir.length()-1 );

    QDir dir( directory );
    QStringList dirList = dir.entryList( QDir::All );
    for( QStringList::Iterator it = dirList.begin(); it != dirList.end(); ++it ) {
        QString file = directory + QString::fromLatin1("/") + *it;
        QFileInfo fi( file );
        if ( (*it) == QString::fromLatin1(".") || (*it) == QString::fromLatin1("..") ||
             (*it) == QString::fromLatin1("ThumbNails") ||
             (*it) == QString::fromLatin1("CategoryImages") ||
             !fi.isReadable() )
            continue;

        if ( fi.isFile() && loadedFiles.find( file ) == 0) {
            QString baseName = file.mid( imageDir.length()+1 );
            if ( ! DB::ImageDB::instance()->isBlocking( baseName ) ) {
                if ( Utilities::canReadImage(file) )
                    _pendingLoad.append( qMakePair( baseName, DB::Image ) );
                else if ( Utilities::isVideo( file ) )
                    _pendingLoad.append( qMakePair( baseName, DB::Video ) );
            }
        }

        else if ( fi.isDir() )  {
            searchForNewFiles( loadedFiles, file );
        }
    }
}

void NewImageFinder::loadExtraFiles()
{
    QProgressDialog  dialog( i18n("<p><b>Loading information from new files</b></p>"
                                  "<p>Depending on the number of images, this may take some time.<br/>"
                                  "However, there is only a delay when new images are found.</p>"),
                             i18n("&Cancel"), _pendingLoad.count() );
    int count = 0;
    ImageInfoList newImages;
    for( LoadList::Iterator it = _pendingLoad.begin(); it != _pendingLoad.end(); ++it, ++count ) {
        dialog.setProgress( count ); // ensure to call setProgress(0)
        qApp->eventLoop()->processEvents( QEventLoop::AllEvents );

        if ( dialog.wasCanceled() )
            return;
        ImageInfoPtr info = loadExtraFile( (*it).first, (*it).second );
        if ( info )
            newImages.append(info);
    }
    DB::ImageDB::instance()->addImages( newImages );
}


ImageInfoPtr NewImageFinder::loadExtraFile( const QString& relativeNewFileName, DB::MediaType type )
{
    QString absoluteNewFileName = Utilities::absoluteImageFileName( relativeNewFileName );
    QString sum = MD5Sum( absoluteNewFileName );
    if ( DB::ImageDB::instance()->md5Map()->contains( sum ) ) {
        QString relativeMatchedFileName = DB::ImageDB::instance()->md5Map()->lookup(sum);
        QString absoluteMatchedFileName = Utilities::absoluteImageFileName( relativeMatchedFileName );
        QFileInfo fi( absoluteMatchedFileName );

        if ( !fi.exists() ) {
            // The file we had a collapse with didn't exists anymore so it is likely moved to this new name
            ImageInfoPtr info = DB::ImageDB::instance()->info( Settings::SettingsData::instance()->imageDirectory() + relativeMatchedFileName );
            if ( !info )
                qWarning("How did that happen? We couldn't find info for the images %s", relativeMatchedFileName.latin1());
            else {
                info->delaySavingChanges(true);
                fi = QFileInfo ( relativeMatchedFileName );
                if ( info->label() == fi.baseName(true) ) {
                    fi = QFileInfo( relativeNewFileName );
                    info->setLabel( fi.baseName(true) );
                }

                info->setFileName( relativeNewFileName );
                info->delaySavingChanges(false);

                // We need to insert the new name into the MD5 map,
                // as it is a map, the value for the moved file will automatically be deleted.
                DB::ImageDB::instance()->md5Map()->insert( sum, info->fileName(true) );

#ifdef HASEXIV2
                Exif::Database::instance()->remove( absoluteMatchedFileName );
                Exif::Database::instance()->add( absoluteNewFileName );
#endif
                return 0;
            }
        }
    }

    ImageInfoPtr info = new ImageInfo( relativeNewFileName, type );
    info->setMD5Sum(sum);
    DB::ImageDB::instance()->md5Map()->insert( sum, info->fileName(true) );
#ifdef HASEXIV2
    Exif::Database::instance()->add( absoluteNewFileName );
#endif

    return info;
}

bool  NewImageFinder::calculateMD5sums( const QStringList& list )
{
    QProgressDialog dialog( i18n("<p><b>Calculating checksum for %1 files<b></p>"
                                 "<p>By storing a checksum for each image KPhotoAlbum is capable of finding images "
                                 "even when you have moved them on the disk.</p>").arg( list.count() ), i18n("&Cancel"), list.count() );

    int count = 0;
    bool dirty = false;

    for( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it, ++count ) {
        ImageInfoPtr info = DB::ImageDB::instance()->info( *it );
        if ( count % 10 == 0 ) {
            dialog.setProgress( count ); // ensure to call setProgress(0)
            qApp->eventLoop()->processEvents( QEventLoop::AllEvents );

            if ( dialog.wasCanceled() )
                return dirty;
        }
        QString md5 = MD5Sum( *it );
        QString orig = info->MD5Sum();
        info->setMD5Sum( md5 );
        if  ( orig != md5 ) {
            dirty = true;
            Utilities::removeThumbNail( *it );
        }

        DB::ImageDB::instance()->md5Map()->insert( md5, info->fileName(true) );
    }
    return dirty;
}

QString NewImageFinder::MD5Sum( const QString& fileName )
{
    QFile file( fileName );
    if ( !file.open( IO_ReadOnly ) ) {
        if ( KMessageBox::warningContinueCancel( 0, i18n("Could not open %1").arg( fileName ) ) == KMessageBox::No )
            return QString::null;
    }

    KMD5 md5calculator( 0 /* char* */);
    md5calculator.reset();
    md5calculator.update( file );
    QString md5 = md5calculator.hexDigest();
    return md5;
}

