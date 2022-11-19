// SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ImageDB.h"

#include "CategoryCollection.h"
#include "MediaCount.h"
#include "NewImageFinder.h"
#include "TagInfo.h"

#include <XMLDB/Database.h>
#include <kpabase/FileName.h>
#include <kpabase/SettingsData.h>
#include <kpabase/UIDelegate.h>

#include <KLocalizedString>
#include <QApplication>
#include <QFileInfo>
#include <QProgressDialog>

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
    return search(m_currentScope, requireOnDisk).files();
}

void ImageDB::markDirty()
{
    Q_EMIT dirty();
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

    Q_EMIT totalChanged(totalCount());
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

    Q_EMIT totalChanged(totalCount());
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
    , m_exifDB(std::make_unique<Exif::Database>(::Settings::SettingsData::instance()->imageDirectory() + QString::fromLatin1("/exif-info.db"), delegate))
    , m_includeFuzzyCounts(false)
    , m_untaggedTag()
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

void ImageDB::setCurrentScope(const ImageSearchInfo &info)
{
    m_currentScope = info;
}

DB::FileName ImageDB::findFirstItemInRange(const DB::FileNameList &images,
                                           const ImageDate &range,
                                           bool includeRanges) const
{
    DB::FileName candidate;
    Utilities::FastDateTime candidateDateStart;
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
    return m_untaggedTag && m_untaggedTag->isValid();
}

Exif::Database *ImageDB::exifDB() const
{
    return m_exifDB.get();
}

const DB::TagInfo *ImageDB::untaggedTag() const
{
    return m_untaggedTag;
}

void ImageDB::setUntaggedTag(DB::TagInfo *tag)
{
    if (m_untaggedTag)
        m_untaggedTag->deleteLater();
    m_untaggedTag = tag;
    if (m_untaggedTag && m_untaggedTag->isValid()) {
        const QSignalBlocker signalBlocker { this };
        Settings::SettingsData::instance()->setUntaggedCategory(m_untaggedTag->categoryName());
        Settings::SettingsData::instance()->setUntaggedTag(m_untaggedTag->tagName());
        connect(Settings::SettingsData::instance(), &Settings::SettingsData::untaggedTagChanged, this, QOverload<const QString &, const QString &>::of(&DB::ImageDB::setUntaggedTag));
    }
}

void ImageDB::setUntaggedTag(const QString &category, const QString &tag)
{
    auto tagInfo = categoryCollection()->categoryForName(category)->itemForName(tag);
    setUntaggedTag(tagInfo);
}

/** \fn void ImageDB::renameCategory( const QString& oldName, const QString newName )
 * \brief Rename category in media items stored in database.
 */

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_ImageDB.cpp"
