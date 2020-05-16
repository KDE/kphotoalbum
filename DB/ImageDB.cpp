/* Copyright (C) 2003-2020 The KPhotoAlbum Development Team

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

#include "CategoryCollection.h"
#include "FileName.h"
#include "MediaCount.h"
#include "NewImageFinder.h"
#include "UIDelegate.h"

#include <Browser/BrowserWidget.h>
#include <XMLDB/Database.h>

#include <KLocalizedString>
#include <QProgressDialog>
#include <qapplication.h>
#include <qfileinfo.h>

using namespace DB;

ImageDB *ImageDB::s_instance = nullptr;

ImageDB *DB::ImageDB::instance()
{
    if (s_instance == nullptr)
        exit(0); // Either we are closing down or ImageDB::instance was called before ImageDB::setup
    return s_instance;
}

void ImageDB::setupXMLDB(const QString &configFile, UIDelegate &delegate)
{
    if (s_instance)
        qFatal("ImageDB::setupXMLDB: Setup must be called only once.");
    s_instance = new XMLDB::Database(configFile, delegate);
    connectSlots();
}

void ImageDB::deleteInstance()
{
    delete s_instance;
    s_instance = nullptr;
}

void ImageDB::connectSlots()
{
    connect(Settings::SettingsData::instance(), QOverload<bool, bool>::of(&Settings::SettingsData::locked), s_instance, &ImageDB::lockDB);
    connect(&s_instance->memberMap(), &MemberMap::dirty, s_instance, &ImageDB::markDirty);
}

QString ImageDB::NONE()
{
    static QString none = QString::fromLatin1("**NONE**");
    return none;
}

DB::FileNameList ImageDB::currentScope(bool requireOnDisk) const
{
    return search(Browser::BrowserWidget::instance()->currentContext(), requireOnDisk).files();
}

void ImageDB::markDirty()
{
    emit dirty();
}

void ImageDB::setDateRange(const ImageDate &range, bool includeFuzzyCounts)
{
    m_selectionRange = range;
    m_includeFuzzyCounts = includeFuzzyCounts;
}

void ImageDB::clearDateRange()
{
    m_selectionRange = ImageDate();
}

void ImageDB::slotRescan()
{
    bool newImages = NewImageFinder().findImages();
    if (newImages)
        markDirty();

    emit totalChanged(totalCount());
}

void ImageDB::slotRecalcCheckSums(const DB::FileNameList &inputList)
{
    DB::FileNameList list = inputList;
    if (list.isEmpty()) {
        list = files();
        md5Map()->clear();
    }

    bool d = NewImageFinder().calculateMD5sums(list, md5Map());
    if (d)
        markDirty();

    emit totalChanged(totalCount());
}

DB::FileNameSet DB::ImageDB::imagesWithMD5Changed()
{
    MD5Map map;
    bool wasCanceled;
    NewImageFinder().calculateMD5sums(files(), &map, &wasCanceled);
    if (wasCanceled)
        return DB::FileNameSet();

    return md5Map()->diff(map);
}

UIDelegate &DB::ImageDB::uiDelegate() const
{
    return m_UI;
}

ImageDB::ImageDB(UIDelegate &delegate)
    : m_UI(delegate)
{
}

DB::MediaCount ImageDB::count(const ImageSearchInfo &searchInfo)
{
    uint images = 0;
    uint videos = 0;
    for (const auto &imageInfo : search(searchInfo)) {
        if (imageInfo->mediaType() == Image)
            ++images;
        else
            ++videos;
    }
    return MediaCount(images, videos);
}

void ImageDB::slotReread(const DB::FileNameList &list, DB::ExifMode mode)
{
    // Do here a reread of the exif info and change the info correctly in the database without loss of previous added data
    QProgressDialog dialog(i18n("Loading information from images"),
                           i18n("Cancel"), 0, list.count());

    uint count = 0;
    for (DB::FileNameList::ConstIterator it = list.begin(); it != list.end(); ++it, ++count) {
        if (count % 10 == 0) {
            dialog.setValue(count); // ensure to call setProgress(0)
            qApp->processEvents(QEventLoop::AllEvents);

            if (dialog.wasCanceled())
                return;
        }

        QFileInfo fi((*it).absolute());

        if (fi.exists())
            info(*it)->readExif(*it, mode);
        markDirty();
    }
}

DB::FileName ImageDB::findFirstItemInRange(const DB::FileNameList &images,
                                           const ImageDate &range,
                                           bool includeRanges) const
{
    DB::FileName candidate;
    QDateTime candidateDateStart;
    for (const DB::FileName &fileName : images) {
        ImageInfoPtr iInfo = info(fileName);

        ImageDate::MatchType match = iInfo->date().isIncludedIn(range);
        if (match == DB::ImageDate::ExactMatch || (includeRanges && match == DB::ImageDate::RangeMatch)) {
            if (candidate.isNull() || iInfo->date().start() < candidateDateStart) {
                candidate = fileName;
                // Looking at this, can't this just be iInfo->date().start()?
                // Just in the middle of refactoring other stuff, so leaving
                // this alone now. TODO(hzeller): revisit.
                candidateDateStart = info(candidate)->date().start();
            }
        }
    }
    return candidate;
}

bool ImageDB::untaggedCategoryFeatureConfigured() const
{
    const auto untaggedCategory = Settings::SettingsData::instance()->untaggedCategory();
    const auto untaggedTag = Settings::SettingsData::instance()->untaggedTag();
    return categoryCollection()->categoryNames().contains(untaggedCategory)
        && categoryCollection()->categoryForName(untaggedCategory)->items().contains(untaggedTag);
}

/** \fn void ImageDB::renameCategory( const QString& oldName, const QString newName )
 * \brief Rename category in media items stored in database.
 */

// vi:expandtab:tabstop=4 shiftwidth=4:
