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
#include "FastDir.h"
#include "Logging.h"
#include "ImageScout.h"

#include <BackgroundJobs/ReadVideoLengthJob.h>
#include <BackgroundJobs/SearchForVideosWithoutVideoThumbnailsJob.h>
#include <BackgroundTaskManager/JobManager.h>
#include <DB/ImageDB.h>
#include <DB/MD5Map.h>
#include <Exif/Database.h>
#include <ImageManager/RawImageDecoder.h>
#include <ImageManager/ThumbnailBuilder.h>
#include <ImageManager/ThumbnailCache.h>
#include <MainWindow/FeatureDialog.h>
#include <MainWindow/Logging.h>
#include <MainWindow/Window.h>
#include <Settings/SettingsData.h>
#include <Utilities/Util.h>

#include <QApplication>
#include <QEventLoop>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QProgressBar>
#include <QProgressDialog>
#include <QStringList>

#include <KLocalizedString>
#include <KMessageBox>
#include <QDataStream>
#include <QFile>
#include <QElapsedTimer>

using namespace DB;

/*****************************************************************
 *
 * NOTES ON PERFORMANCE
 * ===== == ===========
 *
 * - Robert Krawitz <rlk@alum.mit.edu> 2018-05-24
 *
 *
 * GENERAL NOTES ON STORAGE I/O
 * ------- ----- -- ------- ---
 *
 * The two main gates to loading new images are:
 *
 * 1) I/O (how fast can we read images off mass storage)
 *
 *    Different I/O devices have different characteristics in terms of
 *    througput, media latency, and protocol latency.
 *
 *    - Throughput is the raw speed at which data can be transferred,
 *      limited by the physical and/or electronic characteristics of
 *      the medium and the interface.  Short of reducing the amount of
 *      data that's transferred, or clever games with using the most
 *      efficient part of the medium (the outer tracks only for HDD's,
 *      a practice referred to as "short stroking" because it reduces
 *      the distance the head has to seek, at the cost of wasting a
 *      lot of capacity), there's nothing that can be done about this.
 *
 *    - Media latency is the latency component due to characteristics
 *      of the underlying storage medium.  For spinning disks, this is
 *      a function of rotational latency and sek latency.  In some
 *      cases, particularly with hard disks, it is possible to reduce
 *      media latency by arranging to access the data in a way that
 *      reduces seeking.  See DB/FastDir.cpp for an example of this.
 *
 *      While media latency can sometimes be hidden by overlapping
 *      I/O, generally not possible to avoid it.  Sometimes trying too
 *      hard can actually increase media latency if it results in I/O
 *      operations competing against each other requiring additional
 *      seeks.
 *
 *      Overlapping I/O with computation is another matter; that can
 *      easily yield benefit, especially if it eliminates rotational
 *      latency.
 *
 *    - Protocol latency.  This refers to things like SATA overhead,
 *      network overhead (for images stored on a network), and so
 *      forth.  This can encompass multiple things, and often they can
 *      be pipelined by means of multiple queued I/O operations.  For
 *      example, multiple commands can be issued to modern interfaces
 *      (SATA, NVMe) and many network interfaces without waiting for
 *      earlier operations to return.
 *
 *      If protocol latency is high compared with media latency,
 *      having multiple requests outstanding simultaneously can
 *      yield significant benefits.
 *
 *    iostat is a valuable tool for investigating throughput and
 *    looking for possible optimizations.  The IO/sec and data
 *    read/written per second when compared against known media
 *    characteristics (disk and SSD throughput, network bandwidth)
 *    provides valuable information about whether we're getting close
 *    to full performance from the I/O, and user and system CPU time
 *    give us additional clues about whether we're I/O-bound or
 *    CPU-bound.
 *
 *    Historically in the computer field, operations that require
 *    relatively simple processing on large volumes of data are I/O
 *    bound.  But with very fast I/O devices such as NVMe SSDs, some
 *    of which reach 3 GB/sec, that's not always the case.
 *
 * 2) Image (mostly JPEG) loading.
 *
 *    This is a function of image characteristics and image processing
 *    libraries.  Sometimes it's possible to apply parameters to
 *    the underlying image loader to speed it up.  This shows up as user
 *    CPU time.  Usually the only way to improve this performance
 *    characteristic is to use more or faster CPU cores (sometimes GPUs
 *    can assist here) or use better image loading routines (better
 *    libraries).
 *
 *
 * DESCRIPTION OF KPHOTOALBUM IMAGE LOAD PROCESS
 * ----------- -- ----------- ----- ---- -------
 *
 * KPhotoAlbum, when it loads an image, performs three processing steps:
 *
 * 1) Compute the MD5 checksum
 *
 * 2) Extract the Exif metadata
 *
 * 3) Generate a thumbnail
 *
 * Previous to this round of performance tuning, the first two steps
 * were performed in the first pass, and thumbnails were generated in
 * a separate pass.  Assuming that the set of new images is large enough
 * that they cannot all fit in RAM buffers, this results in the I/O
 * being performed twice.  The rewrite results in I/O being performed once.
 *
 * In addition, I have made many other changes:
 *
 * 1) Prior to the MD5 calculation step, a new thread, called a "scout
 *    thread", reads the files into memory.  While this memory is not
 *    directly used in the later computations, it results in the images
 *    being in RAM when they are later needed, making the I/O very fast
 *    (copying data in memory rather than reading it from storage).
 *
 *    This is a way to overlap I/O with computation.
 *
 * 2) The MD5 checksum uses its own I/O to read the data in in larger
 *    chunks than the Qt MD5 routine does.  The Qt routine reads it in
 *    in 4KiB chunks; my experimentation has found that 256KiB chunks
 *    are more efficient, even with a scout thread (it reduces the
 *    number of system calls).
 *
 * 3) When searching for other images to stack with the image being
 *    loaded, the new image loader no longer attempts to determine
 *    whether other candidate filenames are present, nor does it
 *    compute the MD5 checksum of any such files it does find.  Rather,
 *    it only checks for files that are already in KPhotoAlbum, either
 *    previously or as a result of the current load.  Merely checking
 *    for the presence of another file is not cheap, and it's not
 *    necessary; if an image will belong to a stack, we'll either know
 *    it now or when other images that can be stacked are loaded.
 *
 * 4) The Exif metadata extraction is now done only once; previously
 *    it was performed several times at different stages of the loading
 *    process.
 *
 * 5) The thumbnail index is now written out incrementally rather than
 *    the entire index (which can be many megabytes in a large image
 *    database) being rewritten frequently.  The index is fully rewritten
 *    prior to exit.
 *
 *
 * BASELINE PERFORMANCE
 * -------- -----------
 *
 * These measurements were all taken on a Lenovo ThinkPad P70 with 32
 * GB of dual-channel DDR4-2400 DRAM, a Xeon E3-1505M CPU (4 cores/8
 * total hyperthreads, 2.8-3.7 GHz Skylake; usually runs around
 * 3.1-3.2 GHz in practice), a Seagate ST2000LM015-2E8174 2TB HDD, and
 * a Crucial MX300 1TB SATA SSD.  Published numbers and measurements I
 * took otherwise indicate that the HDD can handle about 105-110
 * MB/sec with a maximum of 180 IO/sec (in a favorable case).  The SSD
 * is rated to handle 530 MB/sec read, 510 MB/sec write, 92K random
 * reads/sec, and 83K random writes/sec.
 *
 * The image set I used for all measurements, except as noted,
 * consists of 10839 total files of which about 85% are 20 MP JPEG and
 * the remainder (with a few exceptions are 20 MP RAW files from a
 * Canon EOS 7D mkII camera.  The total dataset is about 92 GB in
 * size.
 *
 * I baselined both drives by reading the same dataset by means of
 *
 * % ls | xargs cat | dd bs=1048576 of=/dev/null
 *
 * The HDD required between 850 and 870 seconds (14'10" to 14'30") to
 * perform this operation, yielding about 105-108 MB/sec.  The SSD
 * achieved about 271 MB/sec, which is well under its rated throughput
 * (hdparm -Tt yields 355 MB/sec, which is likewise nowhere close to
 * its rated throughput).  hdparm -Tt on the HDD yields about 120
 * MB/sec, but throughput to an HDD depends upon which part of the
 * disk is being read.  The outer tracks have a greater angular
 * density to achieve the same linear density (in other words, the
 * circumference of an outer track is longer than that of an inner
 * track, and the data is stored at a constant linear density).  So
 * hdparm isn't very useful on an HDD except as a best case.
 *
 * Note also that hdparm does a single stream read from the device.
 * It does not take advantage of the ability to queue multiple
 * requests.
 * 
 *
 * ANALYSIS OF KPHOTOALBUM LOAD PERFORMANCE
 * -------- -- ----------- ---- -----------
 *
 * I analyzed the following cases, with images stored both on the
 * HDD and the SSD:
 *
 * 1) Images loaded (All, JPEG only, RAW only)
 *
 * B) Thumbnail creation (Including, Excluding)
 *
 * C) Scout threads (0, 1, 2, 3)
 *
 * The JPG image set constitutes 9293 images totaling about 55 GB.  The
 *   JPEG files are mostly 20 MP high quality files, in the range of
 *   6-10 MB.
 * The RAW image set constitutes 1544 images totaling about 37 GB.  The
 *   RAW files are 20 MP files, in the range of 25 MB.
 * The ALL set consists of 10839 or 10840 images totaling about 92 GB
 *   (the above set plus 2 .MOV files and in some cases one additional
 *   JPEG file).
 * 
 * Times are elapsed times; CPU consumption is approximate user+system
 * CPU consumption.  Numbers in parentheses are with thumbnail
 * building disabled.  Note that in the cases with no scout threads on
 * the SSD the times were reproducibly shorter with thumbnail building
 * enabled (reasons are not determined at this time).
 * 
 * Cases building RAW thumbnails generally consumed somewhat more
 * system CPU (in the range of 10-15%) than JPEG-only cases.  This may
 * be due to custom I/O routines used for generating thumbnails with
 * JPEG files; RAW files used the I/O provided by libkdcraw, which
 * uses smaller I/O operations.
 *
 * Estimating CPU time for mixed workloads proved very problematic,
 * as there were significant changes over time.
 * 
 * Elapsed Time
 * ------- ----
 * 
 *                                 SSD                     HDD
 * 
 * JPG - 0 scouts                  4:03 (3:59)
 * JPG - 1 scout                   2:46 (2:44)
 * JPG - 2 scouts                  2:20 (2:07)
 * JPG - 3 scouts                  2:21 (1:58)
 * 
 * ALL - 0 scouts                  6:32 (7:03)            16:01
 * ALL - 1 scout                   4:33 (4:33)            15:01
 * ALL - 2 scouts                  3:37 (3:28)            16:59
 * ALL - 3 scouts                  3:36 (3:15)
 * 
 * RAW - 0 scouts                  2:18 (2:46)
 * RAW - 1 scout                   1:46 (1:46)
 * RAW - 2 scouts                  1:17 (1:17)
 * RAW - 3 scouts                  1:13 (1:13)
 * 
 * User+System CPU
 * ----------- ---
 * 
 *                                 SSD                     HDD
 * 
 * JPG - 0 scouts                  40% (12%)
 * JPG - 1 scout                   70% (20%)
 * JPG - 2 scouts                  85% (15%)
 * JPG - 3 scouts                  85% (15%)
 * 
 * RAW - 0 scouts                  15% (10%)
 * RAW - 1 scout                   18% (12%)
 * RAW - 2 scouts                  25% (15%)
 * RAW - 3 scouts                  25% (15%)
 *
 * I also used kcachegrind to measure CPU consumption on smaller
 * subsets of images (with and without thumbnail creation).  In terms
 * of user CPU consumption, thumbnail creation constitutes the large
 * majority of CPU cycles for processing JPEG files, followed by MD5
 * computation, with Exif parsing lagging far behind.  For RAW files,
 * MD5 computation consumes more cycles, likely in part due to the
 * larger size of RAW files but possibly also related to the smaller
 * filesize of embedded thumbnails (on the Canon 7D mkII, the embedded
 * thumbnail is full size but low quality).
 * 
 * With thumbnail generation:
 * ---- --------- -----------
 * 
 *                                 RAW             JPEG
 * 
 * Thumbnail generation            44%             82%
 *   libjpeg processing              43%             82%
 * MD5 computation                 51%             13%
 * Read Exif                        1%              1.0%
 * 
 * Without thumbnail generation:
 * ------- --------- -----------
 * 
 *                                 RAW             JPEG
 * 
 * MD5 computation                 92%             80%
 * Read Exif                        4%             10%
 *
 *
 * CONCLUSIONS
 * -----------
 *
 * For loading files from hard disk (likely the most common case),
 * there's no reason to consider any loading method other than using a
 * single scout thread and computing thumbnails concurrently.  Even
 * with thumbnail computation, there is very little CPU utilization.
 *
 * Loading from SATA SSD benefits from two scout threads, and possibly
 * more.  For minimal time to regain control, there is some benefit
 * seen from separating thumbnail generation from the rest of the
 * processing stages at the cost of more total elapsed time.  This is
 * more evident with JPEG files than with RAW files in this test case.
 * RAW files typically have smaller thumbnail images which can be
 * extracted and processed more quickly than full-size JPEG files.  On
 * a slower CPU, it may be desirable to return control to the user
 * even if the thumbnails are not built yet.
 *
 * Two other cases would be NVMe (or other very fast) SSDs and network
 * storage.  Since we're seeing evidence of CPU saturation on SATA
 * SSDs, we would likely see this even more strongly with NVMe; with
 * large numbers of images it may be desirable to separate the
 * thumbnail building from the rest of the processing.  It may also be
 * beneficial to use more scout threads.
 *
 * Network storage presents a different problem.  It is likely to have
 * lower throughput -- and certainly much higher latency -- than even
 * HDD, unless the underlying storage medium is SSD and the data is
 * located on a very fast, low latency network.  So there would be no
 * benefit to separating thumbnail processing.  However, due to
 * protocol vs. media latency discussed above, it may well work to use
 * more scout threads.  However, this may saturate the network and the
 * storage, to the detriment of other users, and there's probably no
 * general (or easily discoverable) optimum for this.
 *
 * It's my judgment that most images will be stored on HDDs for at
 * least the next few years, so tuning for that use case is probably
 * the best single choice to be made.
 *
 *****************************************************************/

namespace {
// Number of scout threads for preloading images. More than one scout thread
// yields about 10% less performance with higher IO/sec but lower I/O throughput,
// most probably due to thrashing.
constexpr int IMAGE_SCOUT_THREAD_COUNT = 1;
}

bool NewImageFinder::findImages()
{
    // Load the information from the XML file.
    DB::FileNameSet loadedFiles;

    QElapsedTimer timer;

    timer.start();
    // TODO: maybe the databas interface should allow to query if it
    // knows about an image ? Here we've to iterate through all of them and it
    // might be more efficient do do this in the database without fetching the
    // whole info.
    for ( const DB::FileName& fileName : DB::ImageDB::instance()->images()) {
        loadedFiles.insert(fileName);
    }

    m_pendingLoad.clear();
    searchForNewFiles( loadedFiles, Settings::SettingsData::instance()->imageDirectory() );
    int filesToLoad = m_pendingLoad.count();
    loadExtraFiles();

    qCDebug(TimingLog) << "Loaded " << filesToLoad << " images in " << timer.elapsed() / 1000.0 << " seconds";

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

    // Keep files within a directory more local by processing all files within the
    // directory, and then all subdirectories.
    QStringList subdirList;

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
            subdirList.append( file.absolute() );
        }
    }
    for( QStringList::const_iterator it = subdirList.constBegin(); it != subdirList.constEnd(); ++it )
        searchForNewFiles( loadedFiles, *it );

}

void NewImageFinder::loadExtraFiles()
{
    // FIXME: should be converted to a threadpool for SMP stuff and whatnot :]
    QProgressDialog dialog;
    QElapsedTimer timeSinceProgressUpdate;
    dialog.setLabelText( i18n("<p><b>Loading information from new files</b></p>"
                              "<p>Depending on the number of images, this may take some time.<br/>"
                              "However, there is only a delay when new images are found.</p>") );
    QProgressBar *progressBar = new QProgressBar;
    progressBar->setFormat( QLatin1String("%v/%m") );
    dialog.setBar(progressBar);
    dialog.setMaximum( m_pendingLoad.count() );
    dialog.setMinimumDuration( 1000 );
    QAtomicInt loadedCount = 0;

    setupFileVersionDetection();

    int count = 0;

    ImageScoutQueue asyncPreloadQueue;
    for( LoadList::Iterator it = m_pendingLoad.begin(); it != m_pendingLoad.end(); ++it ) {
        asyncPreloadQueue.enqueue((*it).first);
    }

    ImageScout scout(asyncPreloadQueue, loadedCount, IMAGE_SCOUT_THREAD_COUNT);
    scout.start();

    Exif::Database::instance()->startInsertTransaction();
    dialog.setValue( count ); // ensure to call setProgress(0)
    timeSinceProgressUpdate.start();
    for( LoadList::Iterator it = m_pendingLoad.begin(); it != m_pendingLoad.end(); ++it, ++count ) {
        qApp->processEvents( QEventLoop::AllEvents );

        if ( dialog.wasCanceled() )
        {
            m_pendingLoad.clear();
            Exif::Database::instance()->abortInsertTransaction();
            return;
        }
        // (*it).first: DB::FileName
        // (*it).second: DB::MediaType
        loadExtraFile( (*it).first, (*it).second );
        loadedCount++;          // Atomic
        if ( timeSinceProgressUpdate.elapsed() >= 1000 ) {
            dialog.setValue( count );
            timeSinceProgressUpdate.restart();
        }
    }
    dialog.setValue( count );
    // loadExtraFile() has already inserted all images into the
    // database, but without committing the changes
    DB::ImageDB::instance()->commitDelayedImages();
    Exif::Database::instance()->commitInsertTransaction();

    ImageManager::ThumbnailBuilder::instance()->save();
}

void NewImageFinder::setupFileVersionDetection() {
    // should be cached because loading once per image is expensive
    m_modifiedFileCompString = Settings::SettingsData::instance()->modifiedFileComponent();
    m_modifiedFileComponent = QRegExp(m_modifiedFileCompString);

    m_originalFileComponents << Settings::SettingsData::instance()->originalFileComponent();
    m_originalFileComponents = m_originalFileComponents.at(0).split(QString::fromLatin1(";"));
}

void NewImageFinder::loadExtraFile( const DB::FileName& newFileName, DB::MediaType type )
{
    MD5 sum = MD5Sum( newFileName );
    if ( handleIfImageHasBeenMoved(newFileName, sum) )
        return;

    // check to see if this is a new version of a previous image
    // We'll get the Exif data later, when we get the MD5 checksum.
    ImageInfoPtr info = ImageInfoPtr(new ImageInfo( newFileName, type, false, false ));
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

                MD5 originalSum;
                if (newFileName == originalFileName)
                    originalSum = sum;
                else if (DB::ImageDB::instance()->md5Map()->containsFile( originalFileName ) )
                    originalSum = DB::ImageDB::instance()->md5Map()->lookupFile( originalFileName );
                else
                    // Do *not* attempt to compute the checksum here.  It forces a filesystem
                    // lookup on a file that may not exist and substantially degrades
                    // performance by about 25% on an SSD and about 30% on a spinning disk.
                    // If one of these other files exist, it will be found later in
                    // the image search at which point we'll detect the modified file.
                    continue;
                if ( DB::ImageDB::instance()->md5Map()->contains( originalSum ) ) {
                    // we have a previous copy of this file; copy it's data
                    // from the original.
                    originalInfo = DB::ImageDB::instance()->info( originalFileName );
                    if ( !originalInfo ) {
                        qCDebug(DBLog) << "Original info not found by name for " << originalFileName.absolute() << ", trying by MD5 sum.";
                        originalFileName = DB::ImageDB::instance()->md5Map()->lookup( originalSum );

                        if (!originalFileName.isNull())
                        {
                            qCDebug(DBLog) << "Substitute image " << originalFileName.absolute() << " found.";
                            originalInfo = DB::ImageDB::instance()->info( originalFileName );
                        }

                        if ( !originalInfo )
                        {
                            qCWarning(DBLog,"How did that happen? We couldn't find info for the original image %s; can't copy the original data to %s",
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
    ImageInfoList newImages;
    newImages.append( info );
    DB::ImageDB::instance()->addImages( newImages, false );

    // also inserts image into exif db if present:
    info->setMD5Sum( sum );
    DB::ImageDB::instance()->md5Map()->insert( sum, info->fileName());

    if (originalInfo &&
        Settings::SettingsData::instance()->autoStackNewFiles() ) {

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
    }

    markUnTagged(info);
    ImageManager::ThumbnailBuilder::instance()->buildOneThumbnail( info );
    if ( info->isVideo() && MainWindow::FeatureDialog::hasVideoThumbnailer() ) {
        // needs to be done *after* insertion into database
        BackgroundTaskManager::JobManager::instance()->addJob(
                    new BackgroundJobs::ReadVideoLengthJob(info->fileName(), BackgroundTaskManager::BackgroundVideoPreviewRequest));
    }
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
                qCWarning(DBLog, "How did that happen? We couldn't find info for the images %s", qPrintable(matchedFileName.relative()));
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
                ImageManager::ThumbnailBuilder::instance()->buildOneThumbnail( info );
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

        MD5 md5 = MD5Sum( fileName );
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
