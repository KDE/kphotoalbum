#include "newimagefinder.h"
#include "imagedb.h"
#include <qfileinfo.h>
#include "options.h"
#include "browser.h"
#include <qdir.h>
#include "util.h"
#include <qprogressdialog.h>
#include <klocale.h>
#include <qapplication.h>
#include <qeventloop.h>
#include <kmessagebox.h>
#include <kmdcodec.h>

bool NewImageFinder::findImages()
{
    // Load the information from the XML file.
    QDict<void> loadedFiles( 6301 /* a large prime */ );

    ImageInfoList images = ImageDB::instance()->images();
    for( ImageInfoListIterator it( images ); *it; ++it ) {
        loadedFiles.insert( (*it)->fileName(),
                            (void*)0x1 /* void pointer to nothing I never need the value,
                                          just its existsance, must be != 0x0 though.*/ );
    }

    _pendingLoad.clear();
    searchForNewFiles( loadedFiles, Options::instance()->imageDirectory() );
    loadExtraFiles();

    // To avoid deciding if the new images are shown in a given thumbnail view or in a given search
    // we rather just go to home.
    Browser::instance()->home();
    return (!_pendingLoad.isEmpty()); // returns if new images was found.
}

void NewImageFinder::searchForNewFiles( const QDict<void>& loadedFiles, QString directory )
{
    if ( directory.endsWith( QString::fromLatin1("/") ) )
        directory = directory.mid( 0, directory.length()-1 );

    QString imageDir = Options::instance()->imageDirectory();
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

        if ( fi.isFile() && (loadedFiles.find( file ) == 0) &&
             Util::canReadImage(fi.extension()) ) {
            QString baseName = file.mid( imageDir.length()+1 );

            if ( ! ImageDB::instance()->isBlocking( baseName ) ) {
                _pendingLoad.append( baseName );
            }
        }
        else if ( fi.isDir() )  {
            searchForNewFiles( loadedFiles, file );
        }
    }
}

void NewImageFinder::loadExtraFiles()
{
    QProgressDialog  dialog( i18n("<qt><p><b>Loading information from images</b></p>"
                                  "<p>Depending on the number of images, this may take some time.<br/>"
                                  "However, there is only a delay when new images are found.</p></qt>"),
                             i18n("&Cancel"), _pendingLoad.count() );
    int count = 0;
    ImageInfoList newImages;
    for( QStringList::Iterator it = _pendingLoad.begin(); it != _pendingLoad.end(); ++it, ++count ) {
        dialog.setProgress( count ); // ensure to call setProgress(0)
        qApp->eventLoop()->processEvents( QEventLoop::AllEvents );

        if ( dialog.wasCanceled() )
            return;
        ImageInfo* info = loadExtraFile( *it );
        if ( info )
            newImages.append(info);
    }
    mergeNewImagesInWithExistingList( newImages );
}

void NewImageFinder::mergeNewImagesInWithExistingList( ImageInfoList newImages )
{
    ImageInfoList& images = ImageDB::instance()->images();
    newImages = newImages.sort();
    if ( images.count() == 0 ) {
        // case 1: The existing imagelist is empty.
        images = newImages;
    }
    else if ( newImages.count() == 0 ) {
        // case 2: No images to merge in - that's easy ;-)
    }
    else if ( newImages.first()->startDate().min() > images.last()->startDate().min() ) {
        // case 2: The new list is later than the existsing
        images.appendList(newImages);
    }
    else if ( images.isSorted() ) {
        // case 3: The lists overlaps, and the existsing list is sorted
        images.mergeIn( newImages );
    }
    else{
        // case 4: The lists overlaps, and the existsing list is not sorted in the overlapping range.
        images.appendList( newImages );
    }
}


ImageInfo* NewImageFinder::loadExtraFile( const QString& relativeName )
{
    QString sum = MD5Sum( Options::instance()->imageDirectory() + QString::fromLatin1("/") + relativeName );
    if ( ImageDB::instance()->md5Map()->contains( sum ) ) {
        QString fileName = ImageDB::instance()->md5Map()->lookup(sum);
        QFileInfo fi( Options::instance()->imageDirectory() + QString::fromLatin1("/") + fileName );

        if ( !fi.exists() ) {
            // The file we had a collapse with didn't exists anymore so it is likely moved to this new name

            // Iterate through the db searching for the image with the correct file name
            // PENDING(blackie) Isn't this what XMLDB::find() is all about?
            ImageInfoList images = ImageDB::instance()->images();
            for( ImageInfoListIterator it( images ); *it; ++it ) {
                if ( (*it)->fileName(true) == fileName ) {
                    // Update the label in case it contained the previos file name
                    fi = QFileInfo ( (*it)->fileName() );
                    if ( (*it)->label() == fi.baseName(true) ) {
                        fi = QFileInfo( relativeName );
                        (*it)->setLabel( fi.baseName(true) );
                    }

                    (*it)->setFileName( relativeName );
                    return 0;
                }
            }
        }
    }

    ImageInfo* info = new ImageInfo( relativeName  );
    info->setMD5Sum(sum);
    return info;
}

bool  NewImageFinder::calculateMD5sums( ImageInfoList& list )
{
    QProgressDialog dialog( i18n("<qt><p><b>Calculating checksum of your images<b></p>"
                                 "<p>By storing a checksum for each image KimDaBa is capable of finding images "
                                 "even when you have moved them on the disk.</p></qt>"), i18n("&Cancel"), list.count() );

    int count = 0;
    bool dirty = false;

    for( ImageInfoListIterator it( list ); *it; ++it, ++count ) {
        if ( count % 10 == 0 ) {
            dialog.setProgress( count ); // ensure to call setProgress(0)
            qApp->eventLoop()->processEvents( QEventLoop::AllEvents );

#if QT_VERSION < 0x030104
            if ( dialog.wasCancelled() )
                return dirty;
#else
            if ( dialog.wasCanceled() )
                return dirty;
#endif
        }
        QString md5 = MD5Sum( (*it)->fileName() );
        QString orig = (*it)->MD5Sum();
        (*it)->setMD5Sum( md5 );
        if  ( orig != md5 ) {
            dirty = true;
            Util::removeThumbNail( (*it)->fileName() );
        }

        ImageDB::instance()->md5Map()->insert( md5, (*it)->fileName(true) );
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

