/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "imagedb.h"
#include "showbusycursor.h"
#include "options.h"
#include <qfileinfo.h>
#include <qfile.h>
#include <qdir.h>
#include <kmessagebox.h>
#include <klocale.h>
#include "util.h"
#include "groupCounter.h"
#include <kmdcodec.h>
#include <qprogressdialog.h>
#include <qapplication.h>
#include <qeventloop.h>
#include "browser.h"
#include <qdict.h>
#include "mainview.h"
#include "imageinfo.h"

ImageDB* ImageDB::_instance = 0;

ImageDB::ImageDB( const QDomElement& top, const QDomElement& blockList, bool* dirty )
{
    *dirty = false;

    QString directory = Options::instance()->imageDirectory();
    if ( directory.isEmpty() )
        return;
    if ( directory.endsWith( QString::fromLatin1("/") ) )
        directory = directory.mid( 0, directory.length()-1 );

    // Collect all the ImageInfo's which do not have md5 sum, so we can calculate them in one go,
    // and show the user a progress bar while doing so.
    // This is really only needed for upgrading from KimDaBa version 1.0 so at a later point this
    // code might simply be deleted.
    ImageInfoList missingSums, missingTimes;

    for ( QDomNode node = top.firstChild(); !node.isNull(); node = node.nextSibling() )  {
        QDomElement elm;
        if ( node.isElement() )
            elm = node.toElement();
        else
            continue;

        QString fileName = elm.attribute( QString::fromLatin1("file") );
        if ( fileName.isNull() )
            qWarning( "Element did not contain a file attribute" );
        else {
            ImageInfo* info = load( fileName, elm );
            if ( info->MD5Sum().length()<32 && info->imageOnDisk() )
                missingSums.append( info );
            else
                _md5Map.insert( info->MD5Sum(), fileName );

            // test for valid Timestamps - this should only happen when people upgrade from an older
            // version of KimDaBa, which did not include time stamps.
            if (!info->startDate().hasValidTime())
                missingTimes.append( info );
        }
    }

    // calculate md5sums - this should only happen the first time the user starts kimdaba after md5sum has been introducted.
    if ( missingSums.count() != 0 )  {
        calculateMD5sums( missingSums );
        *dirty = true;
    }

    if ( missingTimes.count() != 0 ) {
        KMessageBox::information(0,i18n("<qt><p>KimDaBa now also shows the time of images."
                                        "As a means of migration for existing KimDaBa users "
                                        "all the time stamps can now be read from your existing images; "
                                        "if you want to do this, use \"Tools->Maintenance->Read EXIF info from files...\" "
                                        "and check \"Read time\". "
                                        "<p>Be aware that this may overwrite the time info you have previously entered. "
                                        "Be sure you have in the current view only the files for which you really want to read time.</p></qt>"),
                                 QString::null,
                                 QString::fromLatin1("showTimeWarningStartup"));
    }

    // Read the block list
    for ( QDomNode node = blockList.firstChild(); !node.isNull(); node = node.nextSibling() )  {
        QDomElement elm;
        if ( node.isElement() )
            elm = node.toElement();
        else
            continue;

        QString fileName = elm.attribute( QString::fromLatin1( "file" ) );
        if ( !fileName.isEmpty() )
            _blockList << fileName;
    }

    *dirty |= (_pendingLoad.count() != 0);

    connect( Options::instance(), SIGNAL( deletedOption( const QString&, const QString& ) ),
             this, SLOT( deleteOption( const QString&, const QString& ) ) );
    connect( Options::instance(), SIGNAL( renamedOption( const QString&, const QString&, const QString& ) ),
             this, SLOT( renameOption( const QString&, const QString&, const QString& ) ) );
    connect( Options::instance(), SIGNAL( locked( bool, bool ) ), this, SLOT( lockDB( bool, bool ) ) );

    checkIfImagesAreSorted();
    checkIfAllImagesHasSizeAttributes();
}

int ImageDB::totalCount() const
{
    return _images.count();
}

void ImageDB::search( const ImageSearchInfo& info, int from, int to )
{
    ShowBusyCursor dummy;
    int c = count( info, true, from, to );
    emit searchCompleted();
    emit matchCountChange( from, to, c );
}

int ImageDB::count( const ImageSearchInfo& info )
{
    return count( info, false, -1, -1 );
}

int ImageDB::count( const ImageSearchInfo& info, bool makeVisible, int from, int to )
{
    int count = 0;
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        bool match = !(*it)->isLocked() && info.match( *it ) && rangeInclude( *it );

        if ( match )
            ++count;

        match &= ( from != -1 && to != -1 && from <= count && count <= to ) ||
                 ( from == -1 && to == -1 );
        if ( makeVisible )
            (*it)->setVisible( match );
    }
    return count;
}

ImageDB* ImageDB::instance()
{
    if ( _instance == 0 )
        qFatal("ImageDB::instance must not be called before ImageDB::setup");
    return _instance;
}

bool ImageDB::setup( const QDomElement& top, const QDomElement& blockList )
{
    bool dirty;
    _instance = new ImageDB( top, blockList, &dirty );
    return dirty;
}

ImageInfo* ImageDB::load( const QString& fileName, QDomElement elm )
{
    ImageInfo* info = new ImageInfo( fileName, elm );
    info->setVisible( false );
    _images.append(info);
    _fileMap.insert( info->fileName(), info );
    return info;
}

void ImageDB::searchForNewFiles( const QDict<void>& loadedFiles, QString directory )
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

            if ( ! _blockList.contains( baseName ) ) {
                _pendingLoad.append( baseName );
            }
        }
        else if ( fi.isDir() )  {
            searchForNewFiles( loadedFiles, file );
        }
    }
}

void ImageDB::loadExtraFiles()
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

void ImageDB::mergeNewImagesInWithExistingList( ImageInfoList newImages )
{
    newImages = newImages.sort();
    if ( _images.count() == 0 ) {
        // case 1: The existing imagelist is empty.
        _images = newImages;
    }
    else if ( newImages.count() == 0 ) {
        // case 2: No images to merge in - that's easy ;-)
    }
    else if ( newImages.first()->startDate().min() > _images.last()->startDate().min() ) {
        // case 2: The new list is later than the existsing
        _images.appendList(newImages);
    }
    else if ( _images.isSorted() ) {
        // case 3: The lists overlaps, and the existsing list is sorted
        _images.mergeIn( newImages );
    }
    else{
        // case 4: The lists overlaps, and the existsing list is not sorted in the overlapping range.
        _images.appendList( newImages );
    }
}


ImageInfo* ImageDB::loadExtraFile( const QString& relativeName )
{
    QString sum = MD5Sum( Options::instance()->imageDirectory() + QString::fromLatin1("/") + relativeName );
    if ( _md5Map.contains( sum ) ) {
        QString fileName = _md5Map[sum];
        QFileInfo fi( Options::instance()->imageDirectory() + QString::fromLatin1("/") + fileName );

        if ( !fi.exists() ) {
            // The file we had a collapse with didn't exists anymore so it is likely moved to this new name

            // Iterate through the db searching for the image with the correct file name
            // PENDING(blackie) Isn't this what ImageDB::find() is all about?
            for( ImageInfoListIterator it( _images ); *it; ++it ) {
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
    _fileMap.insert( info->fileName(), info );
    return info;
}

void ImageDB::save( QDomElement top )
{
    ImageInfoList list = _images;

    // Copy files from clipboard to end of overview, so we don't loose them
    for( ImageInfoListIterator it(_clipboard); *it; ++it ) {
        list.append( *it );
    }

    QDomDocument doc = top.ownerDocument();
    QDomElement images = doc.createElement( QString::fromLatin1( "images" ) );
    top.appendChild( images );

    for( ImageInfoListIterator it( list ); *it; ++it ) {
        images.appendChild( (*it)->save( doc ) );
    }

    QDomElement blockList = doc.createElement( QString::fromLatin1( "blocklist" ) );
    bool any=false;
    for( QStringList::Iterator it = _blockList.begin(); it != _blockList.end(); ++it ) {
        any=true;
        QDomElement elm = doc.createElement( QString::fromLatin1( "block" ) );
        elm.setAttribute( QString::fromLatin1( "file" ), *it );
        blockList.appendChild( elm );
    }

    if (any)
        top.appendChild( blockList );
}

bool ImageDB::isClipboardEmpty()
{
    return _clipboard.count() == 0;
}

QMap<QString,int> ImageDB::classify( const ImageSearchInfo& info, const QString &group )
{
    QMap<QString, int> map;
    GroupCounter counter( group );
    QDict<void> alreadyMatched = findAlreadyMatched( info, group );

    ImageSearchInfo noMatchInfo = info;
    QString currentMatchTxt = noMatchInfo.option( group );
    if ( currentMatchTxt.isEmpty() )
        noMatchInfo.setOption( group, ImageDB::NONE() );
    else
        noMatchInfo.setOption( group, QString::fromLatin1( "%1 & %2" ).arg(currentMatchTxt).arg(ImageDB::NONE()) );

    // Iterate through the whole database of images.
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        bool match = !(*it)->isLocked() && info.match( *it ) && rangeInclude( *it );
        if ( match ) { // If the given image is currently matched.

            // Now iterate through all the categories the current image
            // contains, and increase them in the map mapping from option
            // group to count for this group.
            QStringList list = (*it)->optionValue(group);
            counter.count( list );
            for( QStringList::Iterator it2 = list.begin(); it2 != list.end(); ++it2 ) {
                if ( !alreadyMatched[*it2] ) // We do not want to match "Jesper & Jesper"
                    map[*it2]++;
            }

            // Find those with no other matches
            if ( noMatchInfo.match( *it ) )
                map[ImageDB::NONE()]++;
        }
    }

    QMap<QString,int> groups = counter.result();
    for( QMapIterator<QString,int> it= groups.begin(); it != groups.end(); ++it ) {
        map[it.key()] = it.data();
    }

    return map;
}

ImageInfoList ImageDB::images( const ImageSearchInfo& info, bool onDisk )
{
    ImageInfoList res;
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        bool match = !(*it)->isLocked() && info.match( *it );
        match &= !onDisk || (*it)->imageOnDisk();
        if ( match )
            res.append( *it );
    }
    return res;
}






int ImageDB::countItemsOfCategory( const QString& group )
{
    int count = 0;
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        if ( (*it)->optionValue( group ).count() != 0 )
            ++count;
    }
    return count;
}

void ImageDB::renameOptionGroup( const QString& oldName, const QString newName )
{
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        (*it)->renameOptionGroup( oldName, newName );
    }
}

void ImageDB::blockList( const ImageInfoList& list )
{
    for( ImageInfoListIterator it( list ); *it; ++it) {
        _blockList << (*it)->fileName( true );
        _images.removeRef( *it );
        _fileMap.remove( (*it)->fileName() );
    }
    emit totalChanged( _images.count() );
}

void ImageDB::deleteList( const ImageInfoList& list )
{
    for( ImageInfoListIterator it( list ); *it; ++it ) {
        _images.removeRef( *it );
        _fileMap.remove( (*it)->fileName() );
    }
    emit totalChanged( _images.count() );
}

void ImageDB::renameOption( const QString& category, const QString& oldName, const QString& newName )
{
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        (*it)->renameOption( category, oldName, newName );
    }
}

void ImageDB::deleteOption( const QString& category, const QString& option )
{
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        (*it)->removeOption( category, option );
    }
}

void ImageDB::lockDB( bool lock, bool exclude  )
{
    ImageSearchInfo info = Options::instance()->currentLock();
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        if ( lock ) {
            bool match = info.match( *it );
            if ( !exclude )
                match = !match;
            (*it)->setLocked( match );
        }
        else
            (*it)->setLocked( false );
    }
}

bool  ImageDB::calculateMD5sums( ImageInfoList& list )
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

        _md5Map.insert( md5, (*it)->fileName(true) );
    }
    return dirty;
}

QString ImageDB::MD5Sum( const QString& fileName )
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

void ImageDB::slotRescan()
{
    // Load the information from the XML file.
    QDict<void> loadedFiles( 6301 /* a large prime */ );

    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        QFileInfo fi( (*it)->fileName() );
        bool fileExists = fi.exists();
        (*it)->setImageOnDisk( fileExists );
        if (!fileExists) {
            // we need to delete here the folder value for the images that get deleted
            (*it)->setOption( QString::fromLatin1("Folder"), QStringList() );
        }
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

    emit totalChanged( _images.count() );
}


void ImageDB::slotReread(ImageInfoList rereadList, int mode)
{
    // Do here a reread of the exif info and change the info correctly in the database without loss of previous added data
    QProgressDialog  dialog( i18n("<qt><p><b>Loading time information from images</b></p>"
                                  "<p>Depending on the number of images, this may take some time.</p></qt>"),
                             i18n("Cancel"), rereadList.count() );

    int count=0;
    for( ImageInfoListIterator it( rereadList ); *it; ++it, ++count ) {
        if ( count % 10 == 0 ) {
            dialog.setProgress( count ); // ensure to call setProgress(0)
            qApp->eventLoop()->processEvents( QEventLoop::AllEvents );

            if ( dialog.wasCanceled() )
                return;
        }

        QFileInfo fi( (*it)->fileName() );

        if (fi.exists())
            (*it)->readExif((*it)->fileName(), mode);
        emit dirty();
    }
}


void ImageDB::slotRecalcCheckSums()
{
    _md5Map.clear();
    bool d = calculateMD5sums( _images );
    if ( d )
        emit dirty();

    // To avoid deciding if the new images are shown in a given thumbnail view or in a given search
    // we rather just go to home.
    Browser::instance()->home();

    emit totalChanged( _images.count() );
}

void ImageDB::showUnavailableImages()
{
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        QFileInfo fi( (*it)->fileName() );
        (*it)->setVisible( !fi.exists() );
    }
}

QString ImageDB::NONE()
{
    return i18n("**NONE**");
}

/**
   Returns a list of current context.
   Imagin you have 5000 images, and you search for say Las Vegas, and 1000 images matches that search,
   then you would in the browser see 10 "folders" each with 100 images.
   currentContext() does not only give you whatever seubset of 100 images,
   but does instead give you a list of all 1000 images.
*/
ImageInfoList ImageDB::currentContext( bool onDisk ) const
{
    ImageSearchInfo currentContext = Browser::instance()->currentContext();
    ImageInfoList images;
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        bool match = !(*it)->isLocked() && currentContext.match( *it );
        if ( match && ( !onDisk || (*it)->imageOnDisk() ) )
            images.append(*it);
    }
    return images;
}

void ImageDB::addImage( ImageInfo* info )
{
    _images.append( info );
    _fileMap.insert( info->fileName(), info );
    emit totalChanged( _images.count() );
    emit dirty();
}

ImageInfo* ImageDB::find( const QString& fileName ) const
{
    if ( _fileMap.contains( fileName ) )
        return _fileMap[ fileName ];
    else
        return 0;
}

QDict<void> ImageDB::findAlreadyMatched( const ImageSearchInfo& info, const QString &group )
{
    QDict<void> map;
    QString str = info.option( group );
    if ( str.contains( QString::fromLatin1( "|" ) ) ) {
        return map;
    }

    QStringList list = QStringList::split( QString::fromLatin1( "&" ), str );
    for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        QString nm = (*it).stripWhiteSpace();
        if (! nm.contains( QString::fromLatin1( "!" ) ) )
            map.insert( nm, (void*) 0x1 /* something different from 0x0 */ );
    }
    return map;
}

void ImageDB::checkIfImagesAreSorted()
{
    if ( !KMessageBox::shouldBeShownContinue( QString::fromLatin1( "checkWhetherImagesAreSorted" ) ) )
        return;

    QDateTime last( QDate( 1900, 1, 1 ) );
    bool wrongOrder = false;
    for( ImageInfoListIterator it( _images ); !wrongOrder && *it; ++it ) {
        if ( last > (*it)->startDate().min() )
            wrongOrder = true;
        last = (*it)->startDate().min();
    }

    if ( wrongOrder ) {
        KMessageBox::information( MainView::theMainView(),
                                  i18n("<qt><p>Your images are not sorted, which means that navigating using the date bar "
                                       "will only work suboptimally.</p>"
                                       "<p>In the <b>Maintenance</b> menu, you can find <b>Display Images with Incomplete Dates</b> "
                                       "which you can use to find the images that are missing date information.</p>"
                                       "You can then select the images that you have reason to believe have a correct date "
                                       "in either their EXIF data or on the file, and execute <b>Maintainance->Read EXIF Info</b> "
                                       "to reread the information.</p>"
                                       "<p>Finally, once all images have their dates set, you can execute "
                                       "<b>Images->Sort Selected by Date & Time</b> to sort them in the database.</p></qt>"),
                                  i18n("Images Are Not Sorted"),
                                  QString::fromLatin1( "checkWhetherImagesAreSorted" ) );
    }
}

void ImageDB::setDateRange( const ImageDateRange& range, bool includeFuzzyCounts )
{
    _selectionRange = range;
    _includeFuzzyCounts = includeFuzzyCounts;
}

void ImageDB::clearDateRange()
{
    _selectionRange = ImageDateRange();
}

bool ImageDB::rangeInclude( ImageInfo* info )
{
    if (_selectionRange.start().isNull() )
        return true;

    ImageDateRange::MatchType tp = info->dateRange().isIncludedIn( _selectionRange );
    if ( _includeFuzzyCounts )
        return ( tp == ImageDateRange::ExactMatch || tp == ImageDateRange::RangeMatch );
    else
        return ( tp == ImageDateRange::ExactMatch );
}

void ImageDB::checkIfAllImagesHasSizeAttributes()
{
    QTime time;
    time.start();
    if ( !KMessageBox::shouldBeShownContinue( QString::fromLatin1( "checkWhetherAllImagesIncludesSize" ) ) )
        return;

    if ( ImageInfo::_anyImageWithEmptySize ) {
        KMessageBox::information( MainView::theMainView(),
                                  i18n("<qt><p>Not all the images in the database have information about image sizes; this is needed to "
                                       "get the best result in the thumbnail view. To fix this, simply go to the <tt>Maintainance</tt> menu, and first "
                                       "choose <tt>Remove All Thumbnails</tt>, and after that choose <tt>Build Thumbnails</tt>.</p>"
                                       "<p>Not doing so will result in extra space around images in the thumbnail view - that is all - so "
                                       "there is no urgency in doing it.</p></qt>"),
                                  i18n("Not All Images Have Size Information"),
                                  QString::fromLatin1( "checkWhetherAllImagesIncludesSize" ) );
    }
}

#include "imagedb.moc"
