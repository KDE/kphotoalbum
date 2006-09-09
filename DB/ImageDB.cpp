#include "ImageDB.h"
#include "XMLDB/Database.h"
#include <kinputdialog.h>
#include <klocale.h>
#include <qfileinfo.h>
#include "Browser/BrowserWidget.h"
#include "DB/CategoryCollection.h"
#include <qprogressdialog.h>
#include <qapplication.h>
#include <qeventloop.h>
#include <config.h>
#include "NewImageFinder.h"
#include <DB/MediaCount.h>
#include <kdebug.h>
#ifdef SQLDB_SUPPORT
#include "SQLDB/Database.h"
#include "SQLDB/DatabaseAddress.h"
#endif

using namespace DB;

ImageDB* ImageDB::_instance = 0;

ImageDB* DB::ImageDB::instance()
{
    if ( _instance == 0 )
        qFatal("ImageDB::instance must not be called before ImageDB::setup");
    return _instance;
}

void ImageDB::setupXMLDB( const QString& configFile )
{
    if (_instance)
        qFatal("ImageDB::setupXMLDB: Setup must be called only once.");
    _instance = new XMLDB::Database( configFile );
    connectSlots();
}

#ifdef SQLDB_SUPPORT
void ImageDB::setupSQLDB( const SQLDB::DatabaseAddress& address )
{
    if (_instance)
        qFatal("ImageDB::setupSQLDB: Setup must be called only once.");
    _instance = new SQLDB::Database(address);

    connectSlots();
}
#endif /* SQLDB_SUPPORT */

void ImageDB::connectSlots()
{
    connect( Settings::SettingsData::instance(), SIGNAL( locked( bool, bool ) ), _instance, SLOT( lockDB( bool, bool ) ) );
}

QString ImageDB::NONE()
{
    return i18n("**NONE**");
}

QStringList ImageDB::currentScope( bool requireOnDisk ) const
{
    return search( Browser::BrowserWidget::instance()->currentContext(), requireOnDisk );
}

void ImageDB::setDateRange( const ImageDate& range, bool includeFuzzyCounts )
{
    _selectionRange = range;
    _includeFuzzyCounts = includeFuzzyCounts;
}

void ImageDB::clearDateRange()
{
    _selectionRange = ImageDate();
}

void ImageDB::slotRescan()
{
    bool newImages = NewImageFinder().findImages();
    if ( newImages )
        emit dirty();

    emit totalChanged( totalCount() );
}

void ImageDB::slotRecalcCheckSums( QStringList list )
{
    if ( list.isEmpty() ) {
        list = images();
        md5Map()->clear();
    }

    bool d = NewImageFinder().calculateMD5sums( list );
    if ( d )
        emit dirty();

    // To avoid deciding if the new images are shown in a given thumbnail view or in a given search
    // we rather just go to home.
    Browser::BrowserWidget::instance()->home();

    emit totalChanged( totalCount() );
}

ImageDB::ImageDB()
{
}

DB::MediaCount ImageDB::count( const ImageSearchInfo& searchInfo )
{
    QStringList list = search( searchInfo );
    uint images = 0;
    uint videos = 0;
    for( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
        ImageInfoPtr inf = info( *it );
        if ( inf->mediaType() == Image )
            ++images;
        else
            ++videos;
    }
    return MediaCount( images, videos );
}

void ImageDB::convertBackend(ImageDB* newBackend, QProgressBar* progressBar)
{
    QStringList allImages = images();

    CategoryCollection* origCategories = categoryCollection();
    CategoryCollection* newCategories = newBackend->categoryCollection();

    QValueList<CategoryPtr> categories = origCategories->categories();

    if (progressBar) {
        progressBar->setTotalSteps(categories.count() + allImages.count());
        progressBar->setProgress(0);
    }

    uint n = 0;

    // Convert the Category info
    for( QValueList<CategoryPtr>::ConstIterator it = categories.begin(); it != categories.end(); ++it ) {
        newCategories->addCategory( (*it)->text(), (*it)->iconName(), (*it)->viewType(),
                                    (*it)->thumbnailSize(), (*it)->doShow() );
        newCategories->categoryForName( (*it)->name() )->addOrReorderItems( (*it)->items() );

        if (progressBar) {
            progressBar->setProgress(n++);
            qApp->processEvents();
        }
    }

    // Convert member map
    newBackend->memberMap() = memberMap();

    // Convert all images to the new back end
    uint count = 0;
    ImageInfoList list;
    for( QStringList::ConstIterator it = allImages.begin(); it != allImages.end(); ++it ) {
        list.append( info(*it) );
        if (++count % 100 == 0) {
            newBackend->addImages( list );
            list.clear();
        }
        if (progressBar) {
            progressBar->setProgress(n++);
            qApp->processEvents();
        }
    }
    newBackend->addImages(list);
    if (progressBar)
        progressBar->setProgress(n);
}

void ImageDB::slotReread( const QStringList& list, int mode)
{
    // Do here a reread of the exif info and change the info correctly in the database without loss of previous added data
    QProgressDialog  dialog( i18n("Loading information from images"),
                             i18n("Cancel"), list.count(), 0, "progress dialog", true );

    uint count=0;
    for( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it, ++count  ) {
        if ( count % 10 == 0 ) {
            dialog.setProgress( count ); // ensure to call setProgress(0)
            qApp->eventLoop()->processEvents( QEventLoop::AllEvents );

            if ( dialog.wasCanceled() )
                return;
        }

        QFileInfo fi( *it );

        if (fi.exists())
            info(*it)->readExif(*it, mode);
        emit dirty();
    }

}

QString
ImageDB::findFirstItemInRange(const ImageDate& range,
                              bool includeRanges,
                              const QValueVector<QString>& images) const
{
    QString candidate;
    QDateTime candidateDateStart;
    for (QValueVector<QString>::const_iterator i = images.begin();
         i != images.end(); ++i) {
        ImageInfoPtr iInfo = info(*i);

        ImageDate::MatchType match = iInfo->date().isIncludedIn(range);
        if (match == DB::ImageDate::ExactMatch ||
            (includeRanges && match == DB::ImageDate::RangeMatch)) {
            if (candidate.isNull() ||
                iInfo->date().start() < candidateDateStart) {
                candidate = *i;
                candidateDateStart = info(candidate)->date().start();
            }
        }
    }
    return candidate;
}

/** \fn void ImageDB::renameCategory( const QString& oldName, const QString newName )
 * \brief Rename category in media items stored in database.
 */

#include "ImageDB.moc"
