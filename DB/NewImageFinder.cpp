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
#include "NewImageFinder.h"
#include "ImageManager/ThumbnailBuilder.h"
#include "FastDir.h"
#include "ImageManager/ThumbnailCache.h"

#include "DB/ImageDB.h"
#include "DB/Id.h"
#include "DB/IdList.h"

#include <qfileinfo.h>
#include <QStringList>
#include <QProgressDialog>
#include <klocale.h>
#include <qapplication.h>
#include <qeventloop.h>
#include <kmessagebox.h>
#include "DB/MD5Map.h"

#include "config-kpa-exiv2.h"
#ifdef HAVE_EXIV2
#  include "Exif/Database.h"
#endif

#include "ImageManager/Manager.h"
#include "ImageManager/RawImageDecoder.h"
#include "Settings/SettingsData.h"
#include "Utilities/Util.h"
#include <MainWindow/Window.h>

using namespace DB;

bool NewImageFinder::findImages()
{
    // Load the information from the XML file.
    QSet<QString> loadedFiles;

    // TODO: maybe the databas interface should allow to query if it
    // knows about an image ? Here we've to iterate through all of them and it
    // might be more efficient do do this in the database without fetching the
    // whole info.
    Q_FOREACH( const DB::ImageInfoPtr& info,
        DB::ImageDB::instance()->images().fetchInfos() ) {
        loadedFiles.insert(info->fileName(DB::AbsolutePath));
    }

    _pendingLoad.clear();
    searchForNewFiles( loadedFiles, Settings::SettingsData::instance()->imageDirectory() );
    loadExtraFiles();

    ImageManager::ThumbnailBuilder::instance()->buildMissing();
    // To avoid deciding if the new images are shown in a given thumbnail view or in a given search
    // we rather just go to home.
    return (!_pendingLoad.isEmpty()); // returns if new images was found.
}


void NewImageFinder::searchForNewFiles( const QSet<QString>& loadedFiles, QString directory )
{
    qApp->processEvents( QEventLoop::AllEvents );
    if ( directory.endsWith( QString::fromLatin1("/") ) )
        directory = directory.mid( 0, directory.length()-1 );

    QString imageDir = Settings::SettingsData::instance()->imageDirectory();
    if ( imageDir.endsWith( QString::fromLatin1("/") ) )
        imageDir = imageDir.mid( 0, imageDir.length()-1 );

    FastDir dir( directory );
    QStringList dirList = dir.entryList( );
    ImageManager::RAWImageDecoder dec;   // TODO: DEPENDENCY: DB:: should not reference other directories
    QStringList excluded;
    excluded << Settings::SettingsData::instance()->excludeDirectories();
    excluded = excluded.at(0).split(QString::fromLatin1(","));

    bool skipSymlinks = Settings::SettingsData::instance()->skipSymlinks();

    for( QStringList::const_iterator it = dirList.constBegin(); it != dirList.constEnd(); ++it ) {
        QString file = directory + QString::fromLatin1("/") + *it;
	if ( (*it) == QString::fromLatin1(".") || (*it) == QString::fromLatin1("..") ||
                excluded.contains( (*it) ) || loadedFiles.contains( file ) ||
                dec._skipThisFile(loadedFiles, file) ||
                (*it) == QString::fromLatin1("CategoryImages") )
	    continue;

        QFileInfo fi( file );

	    if ( !fi.isReadable() )
	        continue;
	    if ( skipSymlinks && fi.isSymLink() )
	        continue;

        if ( fi.isFile() ) {
            QString baseName = file.mid( imageDir.length()+1 );
            if ( ! DB::ImageDB::instance()->isBlocking( baseName ) ) {
                if ( Utilities::canReadImage(file) )
                    _pendingLoad.append( qMakePair( baseName, DB::Image ) );
                else if ( Utilities::isVideo( file ) )
                    _pendingLoad.append( qMakePair( baseName, DB::Video ) );
            }
        } else if ( fi.isDir() )  {
            searchForNewFiles( loadedFiles, file );
        }
    }
}

void NewImageFinder::loadExtraFiles()
{
    // FIXME: should be converted to a threadpool for SMP stuff and whatnot :]
    QProgressDialog dialog;
    dialog.setLabelText( i18n("<p><b>Loading information from new files</b></p>"
                              "<p>Depending on the number of images, this may take some time.<br/>"
                              "However, there is only a delay when new images are found.</p>") );
    dialog.setMaximum( _pendingLoad.count() );
    dialog.setMinimumDuration( 1000 );

    setupFileVersionDetection();

    int count = 0;
    ImageInfoList newImages;
    for( LoadList::Iterator it = _pendingLoad.begin(); it != _pendingLoad.end(); ++it, ++count ) {
        dialog.setValue( count ); // ensure to call setProgress(0)
        qApp->processEvents( QEventLoop::AllEvents );

        if ( dialog.wasCanceled() )
            return;
        ImageInfoPtr info = loadExtraFile( (*it).first, (*it).second );
        if ( info ) {
            markUnTagged(info);
            newImages.append(info);
        }
    }
    DB::ImageDB::instance()->addImages( newImages );
}

void NewImageFinder::setupFileVersionDetection() {
    // should be cached because loading once per image is expensive
    _modifiedFileCompString = Settings::SettingsData::instance()->modifiedFileComponent();
    _modifiedFileComponent = QRegExp(_modifiedFileCompString);

    _originalFileComponents << Settings::SettingsData::instance()->originalFileComponent();
    _originalFileComponents = _originalFileComponents.at(0).split(QString::fromLatin1(";"));
}

ImageInfoPtr NewImageFinder::loadExtraFile( const QString& relativeNewFileName, DB::MediaType type )
{
    QString absoluteNewFileName = Utilities::absoluteImageFileName( relativeNewFileName );
    MD5 sum = Utilities::MD5Sum( absoluteNewFileName );
    if ( DB::ImageDB::instance()->md5Map()->contains( sum ) ) {
        QString relativeMatchedFileName = DB::ImageDB::instance()->md5Map()->lookup(sum);
        QString absoluteMatchedFileName = Utilities::absoluteImageFileName( relativeMatchedFileName );
        QFileInfo fi( absoluteMatchedFileName );

        if ( !fi.exists() ) {
            // The file we had a collapse with didn't exists anymore so it is likely moved to this new name
            ImageInfoPtr info = DB::ImageDB::instance()->info( relativeMatchedFileName, DB::RelativeToImageRoot );
            if ( !info )
                qWarning("How did that happen? We couldn't find info for the images %s", qPrintable(relativeMatchedFileName));
            else {
                info->delaySavingChanges(true);
                fi = QFileInfo ( relativeMatchedFileName );
                if ( info->label() == fi.completeBaseName() ) {
                    fi = QFileInfo( relativeNewFileName );
                    info->setLabel( fi.completeBaseName() );
                }

                DB::ImageDB::instance()->renameImage( info, relativeNewFileName );

                // We need to insert the new name into the MD5 map,
                // as it is a map, the value for the moved file will automatically be deleted.
                DB::ImageDB::instance()->md5Map()->insert( sum, info->fileName(DB::RelativeToImageRoot) );

#ifdef HAVE_EXIV2
                Exif::Database::instance()->remove( absoluteMatchedFileName );
                Exif::Database::instance()->add( absoluteNewFileName );
#endif
                return DB::ImageInfoPtr();
            }
        }
    }

    // check to see if this is a new version of a previous image
    ImageInfoPtr info = ImageInfoPtr(new ImageInfo( relativeNewFileName, type ));
    ImageInfoPtr originalInfo;
    QString originalFileName;

    if (Settings::SettingsData::instance()->detectModifiedFiles()) {
        // requires at least *something* in the modifiedFileComponent
        if (_modifiedFileCompString.length() >= 0 &&
            relativeNewFileName.contains(_modifiedFileComponent)) {

            for( QStringList::const_iterator it = _originalFileComponents.constBegin(); 
                 it != _originalFileComponents.constEnd(); ++it ) {
                originalFileName = relativeNewFileName;
                originalFileName.replace(_modifiedFileComponent, (*it));

                MD5 originalSum = Utilities::MD5Sum( Utilities::absoluteImageFileName( originalFileName ) );
                if ( DB::ImageDB::instance()->md5Map()->contains( originalSum ) ) {
                    // we have a previous copy of this file; copy it's data
                    // from the original.
                    originalInfo = DB::ImageDB::instance()->info( originalFileName, DB::RelativeToImageRoot );
                    if ( !originalInfo ) {
                        qDebug() << "Original info not found by name for " << originalFileName << ", trying by MD5 sum.";
                        originalFileName = DB::ImageDB::instance()->md5Map()->lookup( originalSum );

                        if (!originalFileName.isNull())
                        {
                            qDebug() << "Substitute image " << originalFileName << " found.";
                            originalInfo = DB::ImageDB::instance()->info( originalFileName, DB::RelativeToImageRoot );
                        }

                        if ( !originalInfo )
                        {
                            qWarning("How did that happen? We couldn't find info for the original image %s; can't copy the original data to %s", qPrintable(originalFileName), qPrintable(relativeNewFileName));
                            continue;
                        }
                    }
                    info->copyExtraData(*originalInfo);

                    /* if requested to move, then delete old data from original */
                    if (Settings::SettingsData::instance()->moveOriginalContents() ) {
                        originalInfo->removeExtraData();
                    }

                    break;
                }
            }
        }
    }

    // also inserts image into exif db if present:
    info->setMD5Sum(sum);
    DB::ImageDB::instance()->md5Map()->insert( sum, info->fileName(DB::RelativeToImageRoot) );

    if (originalInfo &&
        Settings::SettingsData::instance()->autoStackNewFiles() ) {
        // we have to do this immediately to get the ids
        ImageInfoList newImages;
        newImages.append(info);
        DB::ImageDB::instance()->addImages( newImages );

        // stack the files together
        DB::Id olderfile = DB::ImageDB::instance()->ID_FOR_FILE(originalFileName);
        DB::Id newerfile = DB::ImageDB::instance()->ID_FOR_FILE(info->fileName(DB::AbsolutePath));
        DB::IdList tostack = DB::IdList();

        // the newest file should go to the top of the stack
        tostack.append(newerfile);

        DB::IdList oldStack;
        if ( ( oldStack = DB::ImageDB::instance()->getStackFor( olderfile ) ).isEmpty() ) {
            tostack.append(olderfile);
        } else {
            Q_FOREACH( DB::Id tmp, oldStack ) {
                tostack.append( tmp );
            }
        }
        DB::ImageDB::instance()->stack(tostack);
        MainWindow::Window::theMainWindow()->setStackHead( newerfile );

        // ordering: XXX we ideally want to place the new image right
        // after the older one in the list.

        info = NULL;  // we already added it, so don't process again
    }

    return info;
}

bool  NewImageFinder::calculateMD5sums(
    const DB::IdList& list,
    DB::MD5Map* md5Map,
    bool* wasCanceled)
{
    // FIXME: should be converted to a threadpool for SMP stuff and whatnot :]
    QProgressDialog dialog;
    dialog.setLabelText(
        i18np("<p><b>Calculating checksum for %1 file</b></p>","<p><b>Calculating checksums for %1 files</b></p>", list.size())
		+ i18n("<p>By storing a checksum for each image "
             "KPhotoAlbum is capable of finding images "
             "even when you have moved them on the disk.</p>"));
    dialog.setMaximum(list.size());
    dialog.setMinimumDuration( 1000 );

    int count = 0;
    QStringList cantRead;
    bool dirty = false;

    Q_FOREACH(DB::ImageInfoPtr info, list.fetchInfos()) {
        const QString absoluteFileName = info->fileName(DB::AbsolutePath );
        if ( count % 10 == 0 ) {
            dialog.setValue( count ); // ensure to call setProgress(0)
            qApp->processEvents( QEventLoop::AllEvents );

            if ( dialog.wasCanceled() ) {
                if ( wasCanceled )
                    *wasCanceled = true;
                return dirty;
            }
        }

        MD5 md5 = Utilities::MD5Sum( absoluteFileName );
        if (md5.isNull()) {
            cantRead << absoluteFileName;
            continue;
        }

        if  ( info->MD5Sum() != md5 ) {
            info->setMD5Sum( md5 );
            dirty = true;
            ImageManager::ThumbnailCache::instance()->removeThumbnail( absoluteFileName );
        }

        md5Map->insert( md5, info->fileName(DB::RelativeToImageRoot) );

        ++count;
    }
    if ( wasCanceled )
        *wasCanceled = false;

    if ( !cantRead.empty() )
        KMessageBox::informationList( 0, i18n("Following files could not be read:"), cantRead );

    return dirty;
}

void DB::NewImageFinder::markUnTagged( ImageInfoPtr info )
{
    if ( Settings::SettingsData::instance()->hasUntaggedCategoryFeatureConfigured() ) {
        info->addCategoryInfo( Settings::SettingsData::instance()->untaggedCategory(),
                           Settings::SettingsData::instance()->untaggedTag() );
    }
}
