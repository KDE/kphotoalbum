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

#include "Database.h"

#include "FileReader.h"
#include "FileWriter.h"
#include "Logging.h"
#include "XMLCategory.h"
#include "XMLImageDateCollection.h"

#include <Browser/BrowserWidget.h>
#include <DB/CategoryCollection.h>
#include <DB/FileName.h>
#include <DB/GroupCounter.h>
#include <DB/ImageInfo.h>
#include <DB/ImageInfoPtr.h>
#include <DB/UIDelegate.h>
#include <Exif/Database.h>
#include <MainWindow/Logging.h>
#include <Settings/SettingsData.h>
#include <Utilities/VideoUtil.h>

#include <KLocalizedString>
#include <QElapsedTimer>
#include <QExplicitlySharedDataPointer>
#include <QFileInfo>

using Utilities::StringSet;

namespace
{
void checkForBackupFile(const QString &fileName, DB::UIDelegate &ui)
{
    QString backupName = QFileInfo(fileName).absolutePath() + QString::fromLatin1("/.#") + QFileInfo(fileName).fileName();
    QFileInfo backUpFile(backupName);
    QFileInfo indexFile(fileName);

    if (!backUpFile.exists() || indexFile.lastModified() > backUpFile.lastModified() || backUpFile.size() == 0)
        return;

    const long backupSizeKB = backUpFile.size() >> 10;
    const DB::UserFeedback choice = ui.questionYesNo(
        QString::fromUtf8("Autosave file found: '%1', %2KB.").arg(backupName).arg(backupSizeKB), i18n("Autosave file '%1' exists (size %3 KB) and is newer than '%2'. "
                                                                                                      "Should the autosave file be used?",
                                                                                                      backupName, fileName, backupSizeKB),
        i18n("Found Autosave File"));

    if (choice == DB::UserFeedback::Confirm) {
        qCInfo(XMLDBLog) << "Using autosave file:" << backupName;
        QFile in(backupName);
        if (in.open(QIODevice::ReadOnly)) {
            QFile out(fileName);
            if (out.open(QIODevice::WriteOnly)) {
                char data[1024];
                int len;
                while ((len = in.read(data, 1024)))
                    out.write(data, len);
            }
        }
    }
}
} // namespace

bool XMLDB::Database::s_anyImageWithEmptySize = false;
XMLDB::Database::Database(const QString &configFile, DB::UIDelegate &delegate)
    : ImageDB(delegate)
    , m_fileName(configFile)
{
    checkForBackupFile(configFile, uiDelegate());
    FileReader reader(this);
    reader.read(configFile);
    m_nextStackId = reader.nextStackId();

    connect(categoryCollection(), &DB::CategoryCollection::itemRemoved,
            this, &Database::deleteItem);
    connect(categoryCollection(), &DB::CategoryCollection::itemRenamed,
            this, &Database::renameItem);

    connect(categoryCollection(), &DB::CategoryCollection::itemRemoved,
            &m_members, &DB::MemberMap::deleteItem);
    connect(categoryCollection(), &DB::CategoryCollection::itemRenamed,
            &m_members, &DB::MemberMap::renameItem);
    connect(categoryCollection(), &DB::CategoryCollection::categoryRemoved,
            &m_members, &DB::MemberMap::deleteCategory);
}

uint XMLDB::Database::totalCount() const
{
    return m_images.count();
}

/**
 * I was considering merging the two calls to this method (one for images, one for video), but then I
 * realized that all the work is really done after the check for whether the given
 * imageInfo is of the right type, and as a match can't be both, this really
 * would buy me nothing.
 */
QMap<QString, DB::CountWithRange> XMLDB::Database::classify(const DB::ImageSearchInfo &info, const QString &category, DB::MediaType typemask, DB::ClassificationMode mode)
{
    QElapsedTimer timer;
    timer.start();
    QMap<QString, DB::CountWithRange> map;
    DB::GroupCounter counter(category);
    Utilities::StringSet alreadyMatched = info.findAlreadyMatched(category);

    DB::ImageSearchInfo noMatchInfo = info;
    QString currentMatchTxt = noMatchInfo.categoryMatchText(category);
    if (currentMatchTxt.isEmpty())
        noMatchInfo.setCategoryMatchText(category, DB::ImageDB::NONE());
    else
        noMatchInfo.setCategoryMatchText(category, QString::fromLatin1("%1 & %2").arg(currentMatchTxt).arg(DB::ImageDB::NONE()));
    noMatchInfo.setCacheable(false);

    // Iterate through the whole database of images.
    for (const auto &imageInfo : m_images) {
        bool match = ((imageInfo)->mediaType() & typemask) && !(imageInfo)->isLocked() && info.match(imageInfo) && rangeInclude(imageInfo);
        if (match) { // If the given image is currently matched.

            // Now iterate through all the categories the current image
            // contains, and increase them in the map mapping from category
            // to count.
            StringSet items = (imageInfo)->itemsOfCategory(category);
            counter.count(items, imageInfo->date());
            for (const auto &categoryName : items) {
                if (!alreadyMatched.contains(categoryName)) // We do not want to match "Jesper & Jesper"
                    map[categoryName].add(imageInfo->date());
            }

            // Find those with no other matches
            if (noMatchInfo.match(imageInfo))
                map[DB::ImageDB::NONE()].count++;

            // this is a shortcut for the browser overview page,
            // where we are only interested whether there are sub-categories to a category
            if (mode == DB::ClassificationMode::PartialCount && map.size() > 1) {
                qCInfo(TimingLog) << "Database::classify(partial): " << timer.restart() << "ms.";
                return map;
            }
        }
    }

    QMap<QString, DB::CountWithRange> groups = counter.result();
    for (QMap<QString, DB::CountWithRange>::iterator it = groups.begin(); it != groups.end(); ++it) {
        map[it.key()] = it.value();
    }

    qCInfo(TimingLog) << "Database::classify(): " << timer.restart() << "ms.";
    return map;
}

void XMLDB::Database::renameCategory(const QString &oldName, const QString newName)
{
    for (DB::ImageInfoListIterator it = m_images.begin(); it != m_images.end(); ++it) {
        (*it)->renameCategory(oldName, newName);
    }
}

void XMLDB::Database::addToBlockList(const DB::FileNameList &list)
{
    for (const DB::FileName &fileName : list) {
        m_blockList.insert(fileName);
    }
    deleteList(list);
}

void XMLDB::Database::deleteList(const DB::FileNameList &list)
{
    for (const DB::FileName &fileName : list) {
        const DB::ImageInfoPtr imageInfo = info(fileName);
        StackMap::iterator found = m_stackMap.find(imageInfo->stackId());
        if (imageInfo->isStacked() && found != m_stackMap.end()) {
            const DB::FileNameList origCache = found.value();
            DB::FileNameList newCache;
            for (const DB::FileName &cacheName : origCache) {
                if (fileName != cacheName)
                    newCache.append(cacheName);
            }
            if (newCache.size() <= 1) {
                // we're destroying a stack
                for (const DB::FileName &cacheName : qAsConst(newCache)) {
                    DB::ImageInfoPtr cacheInfo = info(cacheName);
                    cacheInfo->setStackId(0);
                    cacheInfo->setStackOrder(0);
                }
                m_stackMap.remove(imageInfo->stackId());
            } else {
                m_stackMap.insert(imageInfo->stackId(), newCache);
            }
        }
        m_imageCache.remove(imageInfo->fileName().absolute());
        m_images.remove(imageInfo);
    }
    Exif::Database::instance()->remove(list);
    emit totalChanged(m_images.count());
    emit imagesDeleted(list);
    emit dirty();
}

void XMLDB::Database::renameItem(DB::Category *category, const QString &oldName, const QString &newName)
{
    for (DB::ImageInfoListIterator it = m_images.begin(); it != m_images.end(); ++it) {
        (*it)->renameItem(category->name(), oldName, newName);
    }
}

void XMLDB::Database::deleteItem(DB::Category *category, const QString &value)
{
    for (DB::ImageInfoListIterator it = m_images.begin(); it != m_images.end(); ++it) {
        (*it)->removeCategoryInfo(category->name(), value);
    }
}

void XMLDB::Database::lockDB(bool lock, bool exclude)
{
    DB::ImageSearchInfo info = Settings::SettingsData::instance()->currentLock();
    for (DB::ImageInfoListIterator it = m_images.begin(); it != m_images.end(); ++it) {
        if (lock) {
            bool match = info.match(*it);
            if (!exclude)
                match = !match;
            (*it)->setLocked(match);
        } else
            (*it)->setLocked(false);
    }
}

void XMLDB::Database::clearDelayedImages()
{
    m_delayedCache.clear();
    m_delayedUpdate.clear();
}

void XMLDB::Database::forceUpdate(const DB::ImageInfoList &images)
{
    // FIXME: merge stack information
    DB::ImageInfoList newImages = images.sort();
    if (m_images.count() == 0) {
        // case 1: The existing imagelist is empty.
        for (const DB::ImageInfoPtr &imageInfo : qAsConst(newImages))
            m_imageCache.insert(imageInfo->fileName().absolute(), imageInfo);
        m_images = newImages;
    } else if (newImages.count() == 0) {
        // case 2: No images to merge in - that's easy ;-)
        return;
    } else if (newImages.first()->date().start() > m_images.last()->date().start()) {
        // case 2: The new list is later than the existsing
        for (const DB::ImageInfoPtr &imageInfo : qAsConst(newImages))
            m_imageCache.insert(imageInfo->fileName().absolute(), imageInfo);
        m_images.appendList(newImages);
    } else if (m_images.isSorted()) {
        // case 3: The lists overlaps, and the existsing list is sorted
        for (const DB::ImageInfoPtr &imageInfo : qAsConst(newImages))
            m_imageCache.insert(imageInfo->fileName().absolute(), imageInfo);
        m_images.mergeIn(newImages);
    } else {
        // case 4: The lists overlaps, and the existsing list is not sorted in the overlapping range.
        for (const DB::ImageInfoPtr &imageInfo : qAsConst(newImages))
            m_imageCache.insert(imageInfo->fileName().absolute(), imageInfo);
        m_images.appendList(newImages);
    }
}

void XMLDB::Database::addImages(const DB::ImageInfoList &images,
                                bool doUpdate)
{
    for (const DB::ImageInfoPtr &info : images) {
        info->addCategoryInfo(i18n("Media Type"),
                              info->mediaType() == DB::Image ? i18n("Image") : i18n("Video"));
        m_delayedCache.insert(info->fileName().absolute(), info);
        m_delayedUpdate << info;
    }
    if (doUpdate) {
        commitDelayedImages();
    }
}

void XMLDB::Database::commitDelayedImages()
{
    uint imagesAdded = m_delayedUpdate.count();
    if (imagesAdded > 0) {
        forceUpdate(m_delayedUpdate);
        m_delayedCache.clear();
        m_delayedUpdate.clear();
        // It's the responsibility of the caller to add the Exif information.
        // It's more efficient from an I/O perspective to minimize the number
        // of passes over the images, and with the ability to add the Exif
        // data in a transaction, there's no longer any need to read it here.
        emit totalChanged(m_images.count());
        emit dirty();
    }
}

void XMLDB::Database::renameImage(DB::ImageInfoPtr info, const DB::FileName &newName)
{
    info->setFileName(newName);
}

DB::ImageInfoPtr XMLDB::Database::info(const DB::FileName &fileName) const
{
    if (fileName.isNull())
        return DB::ImageInfoPtr();

    const QString name = fileName.absolute();

    if (m_imageCache.contains(name))
        return m_imageCache[name];

    if (m_delayedCache.contains(name))
        return m_delayedCache[name];

    for (const DB::ImageInfoPtr &imageInfo : qAsConst(m_images))
        m_imageCache.insert(imageInfo->fileName().absolute(), imageInfo);

    if (m_imageCache.contains(name)) {
        return m_imageCache[name];
    }

    return DB::ImageInfoPtr();
}

bool XMLDB::Database::rangeInclude(DB::ImageInfoPtr info) const
{
    if (m_selectionRange.start().isNull())
        return true;

    DB::ImageDate::MatchType tp = info->date().isIncludedIn(m_selectionRange);
    if (m_includeFuzzyCounts)
        return (tp == DB::ImageDate::ExactMatch || tp == DB::ImageDate::RangeMatch);
    else
        return (tp == DB::ImageDate::ExactMatch);
}

DB::MemberMap &XMLDB::Database::memberMap()
{
    return m_members;
}

void XMLDB::Database::save(const QString &fileName, bool isAutoSave)
{
    FileWriter saver(this);
    saver.save(fileName, isAutoSave);
}

DB::MD5Map *XMLDB::Database::md5Map()
{
    return &m_md5map;
}

bool XMLDB::Database::isBlocking(const DB::FileName &fileName)
{
    return m_blockList.contains(fileName);
}

DB::FileNameList XMLDB::Database::files() const
{
    return m_images.files();
}

DB::ImageInfoList XMLDB::Database::images() const
{
    return m_images;
}

DB::ImageInfoList XMLDB::Database::search(
    const DB::ImageSearchInfo &info,
    bool requireOnDisk) const
{
    return searchPrivate(info, requireOnDisk, true);
}

DB::ImageInfoList XMLDB::Database::searchPrivate(
    const DB::ImageSearchInfo &info,
    bool requireOnDisk,
    bool onlyItemsMatchingRange) const
{
    // When searching for images counts for the datebar, we want matches outside the range too.
    // When searching for images for the thumbnail view, we only want matches inside the range.
    DB::ImageInfoList result;
    for (DB::ImageInfoListConstIterator it = m_images.constBegin(); it != m_images.constEnd(); ++it) {
        bool match = !(*it)->isLocked() && info.match(*it) && (!onlyItemsMatchingRange || rangeInclude(*it));
        match &= !requireOnDisk || DB::ImageInfo::imageOnDisk((*it)->fileName());

        if (match)
            result.append((*it));
    }
    return result;
}

void XMLDB::Database::sortAndMergeBackIn(const DB::FileNameList &fileNameList)
{
    DB::ImageInfoList infoList;
    for (const DB::FileName &fileName : fileNameList)
        infoList.append(info(fileName));
    m_images.sortAndMergeBackIn(infoList);
}

DB::CategoryCollection *XMLDB::Database::categoryCollection()
{
    return &m_categoryCollection;
}

QExplicitlySharedDataPointer<DB::ImageDateCollection> XMLDB::Database::rangeCollection()
{
    return QExplicitlySharedDataPointer<DB::ImageDateCollection>(
        new XMLImageDateCollection(searchPrivate(Browser::BrowserWidget::instance()->currentContext(), false, false)));
}

void XMLDB::Database::reorder(
    const DB::FileName &item,
    const DB::FileNameList &selection,
    bool after)
{
    Q_ASSERT(!item.isNull());
    DB::ImageInfoList list = takeImagesFromSelection(selection);
    insertList(item, list, after);
}

// Remove all the images from the database that match the given selection and
// return that sublist.
// This returns the selected and erased images in the order in which they appear
// in the image list itself.
DB::ImageInfoList XMLDB::Database::takeImagesFromSelection(const DB::FileNameList &selection)
{
    DB::ImageInfoList result;
    if (selection.isEmpty())
        return result;

    // iterate over all images (expensive!!) TODO: improve?
    for (DB::ImageInfoListIterator it = m_images.begin(); it != m_images.end(); /**/) {
        const DB::FileName imagefile = (*it)->fileName();
        DB::FileNameList::const_iterator si = selection.begin();
        // for each image, iterate over selection, break on match
        for (/**/; si != selection.end(); ++si) {
            const DB::FileName file = *si;
            if (imagefile == file) {
                break;
            }
        }
        // if image is not in selection, simply advance to next, if not add to result and erase
        if (si == selection.end()) {
            ++it;
        } else {
            result << *it;
            m_imageCache.remove((*it)->fileName().absolute());
            it = m_images.erase(it);
        }
        // if all images from selection are in result (size of lists is equal) break.
        if (result.size() == selection.size())
            break;
    }

    return result;
}

void XMLDB::Database::insertList(
    const DB::FileName &fileName,
    const DB::ImageInfoList &list,
    bool after)
{
    DB::ImageInfoListIterator imageIt = m_images.begin();
    for (; imageIt != m_images.end(); ++imageIt) {
        if ((*imageIt)->fileName() == fileName) {
            break;
        }
    }
    // since insert() inserts before iterator increment when inserting AFTER image
    if (after)
        imageIt++;
    for (DB::ImageInfoListConstIterator it = list.begin(); it != list.end(); ++it) {
        // the call to insert() destroys the given iterator so use the new one after the call
        imageIt = m_images.insert(imageIt, *it);
        m_imageCache.insert((*it)->fileName().absolute(), *it);
        // increment always to retain order of selected images
        imageIt++;
    }
    emit dirty();
}

bool XMLDB::Database::stack(const DB::FileNameList &items)
{
    unsigned int changed = 0;
    QSet<DB::StackID> stacks;
    QList<DB::ImageInfoPtr> images;
    unsigned int stackOrder = 1;

    for (const DB::FileName &fileName : items) {
        DB::ImageInfoPtr imgInfo = info(fileName);
        Q_ASSERT(imgInfo);
        if (imgInfo->isStacked()) {
            stacks << imgInfo->stackId();
            stackOrder = qMax(stackOrder, imgInfo->stackOrder() + 1);
        } else {
            images << imgInfo;
        }
    }

    if (stacks.size() > 1)
        return false; // images already in different stacks -> can't stack

    DB::StackID stackId = (stacks.size() == 1) ? *(stacks.begin()) : m_nextStackId++;
    for (DB::ImageInfoPtr info : qAsConst(images)) {
        info->setStackOrder(stackOrder);
        info->setStackId(stackId);
        m_stackMap[stackId].append(info->fileName());
        ++changed;
        ++stackOrder;
    }

    if (changed)
        emit dirty();

    return changed;
}

void XMLDB::Database::unstack(const DB::FileNameList &items)
{
    for (const DB::FileName &fileName : items) {
        const DB::FileNameList allInStack = getStackFor(fileName);
        if (allInStack.size() <= 2) {
            // we're destroying stack here
            for (const DB::FileName &stackFileName : allInStack) {
                DB::ImageInfoPtr imgInfo = info(stackFileName);
                Q_ASSERT(imgInfo);
                if (imgInfo->isStacked()) {
                    m_stackMap.remove(imgInfo->stackId());
                    imgInfo->setStackId(0);
                    imgInfo->setStackOrder(0);
                }
            }
        } else {
            DB::ImageInfoPtr imgInfo = info(fileName);
            Q_ASSERT(imgInfo);
            if (imgInfo->isStacked()) {
                m_stackMap[imgInfo->stackId()].removeAll(fileName);
                imgInfo->setStackId(0);
                imgInfo->setStackOrder(0);
            }
        }
    }

    if (!items.isEmpty())
        emit dirty();
}

DB::FileNameList XMLDB::Database::getStackFor(const DB::FileName &referenceImg) const
{
    DB::ImageInfoPtr imageInfo = info(referenceImg);

    if (!imageInfo || !imageInfo->isStacked())
        return DB::FileNameList();

    StackMap::iterator found = m_stackMap.find(imageInfo->stackId());
    if (found != m_stackMap.end())
        return found.value();

    // it wasn't in the cache -> rebuild it
    m_stackMap.clear();
    for (DB::ImageInfoListConstIterator it = m_images.constBegin(); it != m_images.constEnd(); ++it) {
        if ((*it)->isStacked()) {
            DB::StackID stackid = (*it)->stackId();
            m_stackMap[stackid].append((*it)->fileName());
        }
    }

    found = m_stackMap.find(imageInfo->stackId());
    if (found != m_stackMap.end())
        return found.value();
    else
        return DB::FileNameList();
}

void XMLDB::Database::copyData(const DB::FileName &from, const DB::FileName &to)
{
    (*info(to)).merge(*info(from));
}

int XMLDB::Database::fileVersion()
{
    // File format version, bump it up every time the format for the file changes.
    return 8;
}

// During profiling of loading, I found that a significant amount of time was spent in QDateTime::fromString.
// Reviewing the code, I fount that it did a lot of extra checks we don't need (like checking if the string have
// timezone information (which they won't in KPA), this function is a replacement that is faster than the original.
QDateTime dateTimeFromString(const QString &str)
{
    static QChar T = QChar::fromLatin1('T');

    if (str[10] == T)
        return QDateTime(QDate::fromString(str.left(10), Qt::ISODate), QTime::fromString(str.mid(11), Qt::ISODate));

    else
        return QDateTime::fromString(str, Qt::ISODate);
}

DB::ImageInfoPtr XMLDB::Database::createImageInfo(const DB::FileName &fileName, ReaderPtr reader, Database *db, const QMap<QString, QString> *newToOldCategory)
{
    static QString _label_ = QString::fromUtf8("label");
    static QString _description_ = QString::fromUtf8("description");
    static QString _startDate_ = QString::fromUtf8("startDate");
    static QString _endDate_ = QString::fromUtf8("endDate");
    static QString _yearFrom_ = QString::fromUtf8("yearFrom");
    static QString _monthFrom_ = QString::fromUtf8("monthFrom");
    static QString _dayFrom_ = QString::fromUtf8("dayFrom");
    static QString _hourFrom_ = QString::fromUtf8("hourFrom");
    static QString _minuteFrom_ = QString::fromUtf8("minuteFrom");
    static QString _secondFrom_ = QString::fromUtf8("secondFrom");
    static QString _yearTo_ = QString::fromUtf8("yearTo");
    static QString _monthTo_ = QString::fromUtf8("monthTo");
    static QString _dayTo_ = QString::fromUtf8("dayTo");
    static QString _angle_ = QString::fromUtf8("angle");
    static QString _md5sum_ = QString::fromUtf8("md5sum");
    static QString _width_ = QString::fromUtf8("width");
    static QString _height_ = QString::fromUtf8("height");
    static QString _rating_ = QString::fromUtf8("rating");
    static QString _stackId_ = QString::fromUtf8("stackId");
    static QString _stackOrder_ = QString::fromUtf8("stackOrder");
    static QString _videoLength_ = QString::fromUtf8("videoLength");
    static QString _options_ = QString::fromUtf8("options");
    static QString _0_ = QString::fromUtf8("0");
    static QString _minus1_ = QString::fromUtf8("-1");
    static QString _MediaType_ = i18n("Media Type");
    static QString _Image_ = i18n("Image");
    static QString _Video_ = i18n("Video");

    QString label;
    if (reader->hasAttribute(_label_))
        label = reader->attribute(_label_);
    else
        label = QFileInfo(fileName.relative()).completeBaseName();
    QString description;
    if (reader->hasAttribute(_description_))
        description = reader->attribute(_description_);

    DB::ImageDate date;
    if (reader->hasAttribute(_startDate_)) {
        QDateTime start;

        QString str = reader->attribute(_startDate_);
        if (!str.isEmpty())
            start = dateTimeFromString(str);

        str = reader->attribute(_endDate_);
        if (!str.isEmpty())
            date = DB::ImageDate(start, dateTimeFromString(str));
        else
            date = DB::ImageDate(start);
    } else {
        int yearFrom = 0, monthFrom = 0, dayFrom = 0, yearTo = 0, monthTo = 0, dayTo = 0, hourFrom = -1, minuteFrom = -1, secondFrom = -1;

        yearFrom = reader->attribute(_yearFrom_, _0_).toInt();
        monthFrom = reader->attribute(_monthFrom_, _0_).toInt();
        dayFrom = reader->attribute(_dayFrom_, _0_).toInt();
        hourFrom = reader->attribute(_hourFrom_, _minus1_).toInt();
        minuteFrom = reader->attribute(_minuteFrom_, _minus1_).toInt();
        secondFrom = reader->attribute(_secondFrom_, _minus1_).toInt();

        yearTo = reader->attribute(_yearTo_, _0_).toInt();
        monthTo = reader->attribute(_monthTo_, _0_).toInt();
        dayTo = reader->attribute(_dayTo_, _0_).toInt();
        date = DB::ImageDate(yearFrom, monthFrom, dayFrom, yearTo, monthTo, dayTo, hourFrom, minuteFrom, secondFrom);
    }

    int angle = reader->attribute(_angle_, _0_).toInt();
    DB::MD5 md5sum(reader->attribute(_md5sum_));

    s_anyImageWithEmptySize |= !reader->hasAttribute(_width_);

    int w = reader->attribute(_width_, _minus1_).toInt();
    int h = reader->attribute(_height_, _minus1_).toInt();
    QSize size = QSize(w, h);

    DB::MediaType mediaType = Utilities::isVideo(fileName) ? DB::Video : DB::Image;

    short rating = reader->attribute(_rating_, _minus1_).toShort();
    DB::StackID stackId = reader->attribute(_stackId_, _0_).toULong();
    unsigned int stackOrder = reader->attribute(_stackOrder_, _0_).toULong();

    DB::ImageInfo *info = new DB::ImageInfo(fileName, label, description, date,
                                            angle, md5sum, size, mediaType, rating, stackId, stackOrder);

    if (reader->hasAttribute(_videoLength_))
        info->setVideoLength(reader->attribute(_videoLength_).toInt());

    DB::ImageInfoPtr result(info);

    possibleLoadCompressedCategories(reader, result, db, newToOldCategory);

    while (reader->readNextStartOrStopElement(_options_).isStartToken) {
        readOptions(result, reader, newToOldCategory);
    }

    info->addCategoryInfo(_MediaType_,
                          info->mediaType() == DB::Image ? _Image_ : _Video_);

    return result;
}

void XMLDB::Database::readOptions(DB::ImageInfoPtr info, ReaderPtr reader, const QMap<QString, QString> *newToOldCategory)
{
    static QString _name_ = QString::fromUtf8("name");
    static QString _value_ = QString::fromUtf8("value");
    static QString _option_ = QString::fromUtf8("option");
    static QString _area_ = QString::fromUtf8("area");

    while (reader->readNextStartOrStopElement(_option_).isStartToken) {
        QString name = FileReader::unescape(reader->attribute(_name_));
        // If the silent update to db version 6 has been done, use the updated category names.
        if (newToOldCategory) {
            name = newToOldCategory->key(name, name);
        }

        if (!name.isNull()) {
            // Read values
            while (reader->readNextStartOrStopElement(_value_).isStartToken) {
                QString value = reader->attribute(_value_);

                if (reader->hasAttribute(_area_)) {
                    QStringList areaData = reader->attribute(_area_).split(QString::fromUtf8(" "));
                    int x = areaData[0].toInt();
                    int y = areaData[1].toInt();
                    int w = areaData[2].toInt();
                    int h = areaData[3].toInt();
                    QRect area = QRect(QPoint(x, y), QPoint(x + w - 1, y + h - 1));

                    if (!value.isNull()) {
                        info->addCategoryInfo(name, value, area);
                    }
                } else {
                    if (!value.isNull()) {
                        info->addCategoryInfo(name, value);
                    }
                }
                reader->readEndElement();
            }
        }
    }
}

void XMLDB::Database::possibleLoadCompressedCategories(ReaderPtr reader, DB::ImageInfoPtr info, Database *db, const QMap<QString, QString> *newToOldCategory)
{
    if (db == nullptr)
        return;

    const auto categories = db->m_categoryCollection.categories();
    for (const DB::CategoryPtr &categoryPtr : categories) {
        const QString categoryName = categoryPtr->name();
        QString oldCategoryName;
        if (newToOldCategory) {
            // translate to old categoryName, defaulting to the original name if not found:
            oldCategoryName = newToOldCategory->value(categoryName, categoryName);
        } else {
            oldCategoryName = categoryName;
        }
        QString str = reader->attribute(FileWriter::escape(oldCategoryName));
        if (!str.isEmpty()) {
            const QStringList list = str.split(QString::fromLatin1(","), QString::SkipEmptyParts);
            for (const QString &tagString : list) {
                int id = tagString.toInt();
                if (id != 0 || categoryPtr->isSpecialCategory()) {
                    const QString name = static_cast<const XMLCategory *>(categoryPtr.data())->nameForId(id);
                    info->addCategoryInfo(categoryName, name);
                } else {
                    QStringList tags = static_cast<const XMLCategory *>(categoryPtr.data())->namesForId(id);
                    if (tags.size() == 1) {
                        qCInfo(XMLDBLog) << "Fixing tag " << categoryName << "/" << tags[0] << "with id=0 for image" << info->fileName().relative();
                    } else {
                        // insert marker category
                        QString markerTag = i18n("KPhotoAlbum - manual repair needed (%1)",
                                                 tags.join(i18nc("Separator in a list of tags", ", ")));
                        categoryPtr->addItem(markerTag);
                        info->addCategoryInfo(categoryName, markerTag);
                        qCWarning(XMLDBLog) << "Manual fix required for image" << info->fileName().relative();
                        qCWarning(XMLDBLog) << "Image was marked with tag " << categoryName << "/" << markerTag;
                    }
                    for (const auto &name : tags) {
                        info->addCategoryInfo(categoryName, name);
                    }
                }
            }
        }
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:
