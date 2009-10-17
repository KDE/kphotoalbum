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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "ImageDB.h"
#include "XMLDB/Database.h"
#include <klocale.h>
#include <qfileinfo.h>
#include <QList>
#include "Browser/BrowserWidget.h"
#include "DB/CategoryCollection.h"
#include "DB/ResultId.h"
#include <QProgressBar>
#include <qapplication.h>
#include "NewImageFinder.h"
#include <DB/MediaCount.h>
#include <config-kpa-sqldb.h>
#ifdef SQLDB_SUPPORT
#include "SQLDB/Database.h"
#include "SQLDB/DatabaseAddress.h"
#endif
#include <QProgressDialog>

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

void ImageDB::deleteInstance()
{
    delete _instance;
    _instance = 0;
}

void ImageDB::connectSlots()
{
    connect( Settings::SettingsData::instance(), SIGNAL( locked( bool, bool ) ), _instance, SLOT( lockDB( bool, bool ) ) );
    connect( &_instance->memberMap(), SIGNAL( dirty() ), _instance, SLOT( markDirty() ));
}

QString ImageDB::NONE()
{
    return QString::fromLatin1("**NONE**");
}

DB::Result ImageDB::currentScope(bool requireOnDisk) const
{
    // TODO: DEPENDENCY: DB:: should not depend on other directories.
    return search( Browser::BrowserWidget::instance()->currentContext(), requireOnDisk );
}

void ImageDB::markDirty()
{
    emit dirty();
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
        markDirty();

    emit totalChanged( totalCount() );
}

void ImageDB::slotRecalcCheckSums(const DB::Result& inputList)
{
    DB::Result list = inputList;
    if (list.isEmpty()) {
        list = images();
        md5Map()->clear();
    }

    bool d = NewImageFinder().calculateMD5sums( list, md5Map() );
    if ( d )
        markDirty();

    emit totalChanged( totalCount() );
}

StringSet DB::ImageDB::imagesWithMD5Changed()
{
    MD5Map map;
    bool wasCanceled;
    NewImageFinder().calculateMD5sums(images(), &map, &wasCanceled);
    if ( wasCanceled )
        return StringSet();

    StringSet changes =  md5Map()->diff( map );
    StringSet res;
    for ( StringSet::const_iterator it = changes.begin(); it != changes.end(); ++it )
        res.insert( Settings::SettingsData::instance()->imageDirectory() + *it );
    return res;

}


ImageDB::ImageDB()
{
}

DB::MediaCount ImageDB::count( const ImageSearchInfo& searchInfo )
{
    uint images = 0;
    uint videos = 0;
    Q_FOREACH(const DB::ImageInfoPtr inf, search(searchInfo).fetchInfos()) {
        if ( inf->mediaType() == Image )
            ++images;
        else
            ++videos;
    }
    return MediaCount( images, videos );
}

void ImageDB::convertBackend(ImageDB* newBackend, QProgressBar* progressBar)
{
    const DB::Result allImages = images();

    CategoryCollection* origCategories = categoryCollection();
    CategoryCollection* newCategories = newBackend->categoryCollection();

     QList<CategoryPtr> categories = origCategories->categories();

    if (progressBar) {
        progressBar->setMaximum(categories.count() + allImages.size());
        progressBar->setValue(0);
    }

    uint n = 0;

    // Convert the Category info
     for( QList<CategoryPtr>::ConstIterator it = categories.constBegin(); it != categories.constEnd(); ++it ) {
        newCategories->addCategory( (*it)->name(), (*it)->iconName(), (*it)->viewType(),
                                    (*it)->thumbnailSize(), (*it)->doShow() );
        newCategories->categoryForName( (*it)->name() )->addOrReorderItems( (*it)->items() );

        if (progressBar) {
            progressBar->setValue(n++);
            qApp->processEvents();
        }
    }

    // Convert member map
    newBackend->memberMap() = memberMap();

    // Convert all images to the new back end
    uint count = 0;
    ImageInfoList list;
    Q_FOREACH(const DB::ImageInfoPtr info, allImages.fetchInfos()) {
        list.append(info);
        if (++count % 100 == 0) {
            newBackend->addImages( list );
            list.clear();
        }
        if (progressBar) {
            progressBar->setValue(n++);
            qApp->processEvents();
        }
    }
    newBackend->addImages(list);
    if (progressBar)
        progressBar->setValue(n);
}

void ImageDB::slotReread( const QStringList& list, DB::ExifMode mode)
{
// Do here a reread of the exif info and change the info correctly in the database without loss of previous added data
    QProgressDialog  dialog( i18n("Loading information from images"),
                             i18n("Cancel"), 0, list.count() );

    uint count=0;
    for( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it, ++count  ) {
        if ( count % 10 == 0 ) {
            dialog.setValue( count ); // ensure to call setProgress(0)
            qApp->processEvents( QEventLoop::AllEvents );

            if ( dialog.wasCanceled() )
                return;
        }

        QFileInfo fi( *it );

        if (fi.exists())
            info(*it, DB::AbsolutePath)->readExif(*it, mode);
        markDirty();
    }
}

DB::ResultId ImageDB::findFirstItemInRange(const DB::Result& images,
                                           const ImageDate& range,
                                           bool includeRanges) const
{
    DB::ResultId candidate;
    QDateTime candidateDateStart;
    Q_FOREACH(DB::ResultId id, images) {
        ImageInfoPtr iInfo = id.fetchInfo();

        ImageDate::MatchType match = iInfo->date().isIncludedIn(range);
        if (match == DB::ImageDate::ExactMatch ||
            (includeRanges && match == DB::ImageDate::RangeMatch)) {
            if (candidate.isNull() ||
                iInfo->date().start() < candidateDateStart) {
                candidate = id;
                // Looking at this, can't this just be iInfo->date().start()?
                // Just in the middle of refactoring other stuff, so leaving
                // this alone now. TODO(hzeller): revisit.
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
