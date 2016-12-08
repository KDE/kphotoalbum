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
#include <qfileinfo.h>
#include <QStringList>
#include <QProgressDialog>
#include <KLocalizedString>
#include <qapplication.h>
#include <qeventloop.h>
#include <kmessagebox.h>
#include "DB/MD5Map.h"

#include "Exif/Database.h"

#include "ImageManager/RawImageDecoder.h"
#include "Settings/SettingsData.h"
#include "Utilities/Util.h"
#include <MainWindow/Window.h>
#include <MainWindow/FeatureDialog.h>
#include <BackgroundTaskManager/JobManager.h>
#include <BackgroundJobs/ReadVideoLengthJob.h>
#include <BackgroundJobs/SearchForVideosWithoutVideoThumbnailsJob.h>
#include <QDebug>

using namespace DB;

bool NewImageFinder::findImages()
{
    // Load the information from the XML file.
    DB::FileNameSet loadedFiles;

    // TODO: maybe the databas interface should allow to query if it
    // knows about an image ? Here we've to iterate through all of them and it
    // might be more efficient do do this in the database without fetching the
    // whole info.
    for ( const DB::FileName& fileName : DB::ImageDB::instance()->images()) {
        loadedFiles.insert(fileName);
    }

    m_pendingLoad.clear();
    searchForNewFiles( loadedFiles, Settings::SettingsData::instance()->imageDirectory() );
    loadExtraFiles();

    // Only build thumbnails for the newly found images
    if (! m_pendingLoad.isEmpty()) {
        DB::FileNameList thumbnailsToBuild;

        QListIterator<QPair<DB::FileName, DB::MediaType>> newFiles(m_pendingLoad);
        while (newFiles.hasNext()) {
            QPair<DB::FileName, DB::MediaType> newFile = newFiles.next();
            thumbnailsToBuild << newFile.first;
        }

        ImageManager::ThumbnailBuilder::instance()->scheduleThumbnailBuild(
                thumbnailsToBuild, ImageManager::ThumbnailBuildStart::StartNow
                );
    }

    // Man this is not super optimal, but will be changed onces the image finder moves to become a background task.
    if ( MainWindow::FeatureDialog::hasVideoThumbnailer() ) {
        BackgroundTaskManager::JobManager::instance()->addJob(
                new BackgroundJobs::SearchForVideosWithoutVideoThumbnailsJob );
    }

    // To avoid deciding if the new images are shown in a given thumbnail view or in a given search
    // we rather just go to home.
    return (!m_pendingLoad.isEmpty()); // returns if new images was found.
}


void NewImageFinder::searchForNewFiles( const DB::FileNameSet& loadedFiles, QString directory )
{
    qApp->processEvents( QEventLoop::AllEvents );
    directory = Utilities::stripEndingForwardSlash(directory);

    const QString imageDir = Utilities::stripEndingForwardSlash(Settings::SettingsData::instance()->imageDirectory());

    FastDir dir( directory );
    const QStringList dirList = dir.entryList( );
    ImageManager::RAWImageDecoder dec;
    QStringList excluded;
    excluded << Settings::SettingsData::instance()->excludeDirectories();
    excluded = excluded.at(0).split(QString::fromLatin1(","));

    bool skipSymlinks = Settings::SettingsData::instance()->skipSymlinks();

    for( QStringList::const_iterator it = dirList.constBegin(); it != dirList.constEnd(); ++it )
    {
        const DB::FileName file = DB::FileName::fromAbsolutePath(directory + QString::fromLatin1("/") + *it);
        if ( (*it) == QString::fromLatin1(".") || (*it) == QString::fromLatin1("..") ||
             excluded.contains( (*it) ) || loadedFiles.contains( file ) ||
             dec._skipThisFile(loadedFiles, file) ||
             (*it) == QString::fromLatin1("CategoryImages") )
            continue;

        QFileInfo fi( file.absolute() );

        if ( !fi.isReadable() )
            continue;
        if ( skipSymlinks && fi.isSymLink() )
            continue;

        if ( fi.isFile() ) {
            if ( ! DB::ImageDB::instance()->isBlocking( file ) ) {
                if ( Utilities::canReadImage(file) )
                    m_pendingLoad.append( qMakePair( file, DB::Image ) );
                else if ( Utilities::isVideo( file ) )
                    m_pendingLoad.append( qMakePair( file, DB::Video ) );
            }
        } else if ( fi.isDir() )  {
            searchForNewFiles( loadedFiles, file.absolute() );
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
    dialog.setMaximum( m_pendingLoad.count() );
    dialog.setMinimumDuration( 1000 );

    setupFileVersionDetection();

    int count = 0;
    ImageInfoList newImages;
    for( LoadList::Iterator it = m_pendingLoad.begin(); it != m_pendingLoad.end(); ++it, ++count ) {
        dialog.setValue( count ); // ensure to call setProgress(0)
        qApp->processEvents( QEventLoop::AllEvents );

        if ( dialog.wasCanceled() )
        {
            // clear the list of pending images, so that findImages() doesn't
            // try to build thumbnails for images w/o DB entry:
            m_pendingLoad.clear();
            return;
        }
        // (*it).first: DB::FileName
        // (*it).second: DB::MediaType
        ImageInfoPtr info = loadExtraFile( (*it).first, (*it).second );
        if ( info ) {
            markUnTagged(info);
            newImages.append(info);
        }
    }
    DB::ImageDB::instance()->addImages( newImages );

    // I would have loved to do this in loadExtraFile, but the image has not been added to the database yet
    if ( MainWindow::FeatureDialog::hasVideoThumbnailer() ) {
        Q_FOREACH( const ImageInfoPtr& info, newImages ) {
            if ( info->isVideo() )
                BackgroundTaskManager::JobManager::instance()->addJob(
                        new BackgroundJobs::ReadVideoLengthJob(info->fileName(), BackgroundTaskManager::BackgroundVideoPreviewRequest));
        }
    }

}

void NewImageFinder::setupFileVersionDetection() {
    // should be cached because loading once per image is expensive
    m_modifiedFileCompString = Settings::SettingsData::instance()->modifiedFileComponent();
    m_modifiedFileComponent = QRegExp(m_modifiedFileCompString);

    m_originalFileComponents << Settings::SettingsData::instance()->originalFileComponent();
    m_originalFileComponents = m_originalFileComponents.at(0).split(QString::fromLatin1(";"));
}

ImageInfoPtr NewImageFinder::loadExtraFile( const DB::FileName& newFileName, DB::MediaType type )
{
    MD5 sum = Utilities::MD5Sum( newFileName );
    if ( handleIfImageHasBeenMoved(newFileName, sum) )
        return DB::ImageInfoPtr();

    // check to see if this is a new version of a previous image
    ImageInfoPtr info = ImageInfoPtr(new ImageInfo( newFileName, type ));
    ImageInfoPtr originalInfo;
    DB::FileName originalFileName;

    if (Settings::SettingsData::instance()->detectModifiedFiles()) {
        // requires at least *something* in the modifiedFileComponent
        if (m_modifiedFileCompString.length() >= 0 &&
            newFileName.relative().contains(m_modifiedFileComponent)) {

            for( QStringList::const_iterator it = m_originalFileComponents.constBegin();
                 it != m_originalFileComponents.constEnd(); ++it ) {
                QString tmp = newFileName.relative();
                tmp.replace(m_modifiedFileComponent, (*it));
                originalFileName = DB::FileName::fromRelativePath(tmp);

                MD5 originalSum = Utilities::MD5Sum( originalFileName );
                if ( DB::ImageDB::instance()->md5Map()->contains( originalSum ) ) {
                    // we have a previous copy of this file; copy it's data
                    // from the original.
                    originalInfo = DB::ImageDB::instance()->info( originalFileName );
                    if ( !originalInfo ) {
                        qDebug() << "Original info not found by name for " << originalFileName.absolute() << ", trying by MD5 sum.";
                        originalFileName = DB::ImageDB::instance()->md5Map()->lookup( originalSum );

                        if (!originalFileName.isNull())
                        {
                            qDebug() << "Substitute image " << originalFileName.absolute() << " found.";
                            originalInfo = DB::ImageDB::instance()->info( originalFileName );
                        }

                        if ( !originalInfo )
                        {
                            qWarning("How did that happen? We couldn't find info for the original image %s; can't copy the original data to %s",
                                     qPrintable(originalFileName.absolute()), qPrintable(newFileName.absolute()));
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
    DB::ImageDB::instance()->md5Map()->insert( sum, info->fileName());

    if (originalInfo &&
        Settings::SettingsData::instance()->autoStackNewFiles() ) {
        // we have to do this immediately to get the ids
        ImageInfoList newImages;
        newImages.append(info);
        DB::ImageDB::instance()->addImages( newImages );

        // stack the files together
        DB::FileName olderfile = originalFileName;
        DB::FileName newerfile = info->fileName();
        DB::FileNameList tostack;

        // the newest file should go to the top of the stack
        tostack.append(newerfile);

        DB::FileNameList oldStack;
        if ( ( oldStack = DB::ImageDB::instance()->getStackFor( olderfile)).isEmpty() ) {
            tostack.append(olderfile);
        } else {
            for ( const DB::FileName& tmp : oldStack ) {
                tostack.append( tmp );
            }
        }
        DB::ImageDB::instance()->stack(tostack);
        MainWindow::Window::theMainWindow()->setStackHead(newerfile);

        // ordering: XXX we ideally want to place the new image right
        // after the older one in the list.

        info = nullptr;  // we already added it, so don't process again
    }

    return info;
}

bool NewImageFinder::handleIfImageHasBeenMoved(const FileName &newFileName, const MD5& sum)
{
    if ( DB::ImageDB::instance()->md5Map()->contains( sum ) ) {
        const DB::FileName matchedFileName = DB::ImageDB::instance()->md5Map()->lookup(sum);
        QFileInfo fi( matchedFileName.absolute() );

        if ( !fi.exists() ) {
            // The file we had a collapse with didn't exists anymore so it is likely moved to this new name
            ImageInfoPtr info = DB::ImageDB::instance()->info( matchedFileName);
            if ( !info )
                qWarning("How did that happen? We couldn't find info for the images %s", qPrintable(matchedFileName.relative()));
            else {
                info->delaySavingChanges(true);
                fi = QFileInfo ( matchedFileName.relative() );
                if ( info->label() == fi.completeBaseName() ) {
                    fi = QFileInfo( newFileName.absolute() );
                    info->setLabel( fi.completeBaseName() );
                }

                DB::ImageDB::instance()->renameImage( info, newFileName );

                // We need to insert the new name into the MD5 map,
                // as it is a map, the value for the moved file will automatically be deleted.

                DB::ImageDB::instance()->md5Map()->insert( sum, info->fileName());

                Exif::Database::instance()->remove( matchedFileName );
                Exif::Database::instance()->add( newFileName);
                return true;
            }
        }
    }
    return false; // The image wasn't just moved
}

bool  NewImageFinder::calculateMD5sums(
    const DB::FileNameList& list,
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
    DB::FileNameList cantRead;
    bool dirty = false;

    for (const FileName& fileName : list) {
        if ( count % 10 == 0 ) {
            dialog.setValue( count ); // ensure to call setProgress(0)
            qApp->processEvents( QEventLoop::AllEvents );

            if ( dialog.wasCanceled() ) {
                if ( wasCanceled )
                    *wasCanceled = true;
                return dirty;
            }
        }

        MD5 md5 = Utilities::MD5Sum( fileName );
        if (md5.isNull()) {
            cantRead << fileName;
            continue;
        }

        ImageInfoPtr info = ImageDB::instance()->info(fileName);
        if  ( info->MD5Sum() != md5 ) {
            info->setMD5Sum( md5 );
            dirty = true;
            ImageManager::ThumbnailCache::instance()->removeThumbnail(fileName);
        }

        md5Map->insert( md5, fileName );

        ++count;
    }
    if ( wasCanceled )
        *wasCanceled = false;

    if ( !cantRead.empty() )
        KMessageBox::informationList( nullptr, i18n("Following files could not be read:"), cantRead.toStringList(DB::RelativeToImageRoot) );

    return dirty;
}

void DB::NewImageFinder::markUnTagged( ImageInfoPtr info )
{
    if ( Settings::SettingsData::instance()->hasUntaggedCategoryFeatureConfigured() ) {
        info->addCategoryInfo( Settings::SettingsData::instance()->untaggedCategory(),
                           Settings::SettingsData::instance()->untaggedTag() );
    }
}
// vi:expandtab:tabstop=4 shiftwidth=4:
