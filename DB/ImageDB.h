// SPDX-FileCopyrightText: 2003-2014 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2006-2010 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2007-2008 Jan Kundrát <jkt@flaska.net>
// SPDX-FileCopyrightText: 2008-2009 Henner Zeller <h.zeller@acm.org>
// SPDX-FileCopyrightText: 2012-2015 Miika Turkia <miika.turkia@gmail.com>
// SPDX-FileCopyrightText: 2012-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2015-2022 Tobias Leupold <tl@stonemx.de>
// SPDX-FileCopyrightText: 2018 Robert Krawitz <rlk@alum.mit.edu>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IMAGEDB_H
#define IMAGEDB_H

#include "Category.h"
#include "ImageInfoList.h"
#include "ImageInfoPtr.h"
#include "MediaCount.h"

#include <DB/CategoryCollection.h>
#include <DB/MD5Map.h>
#include <DB/MemberMap.h>
#include <DB/XML/FileReader.h>
#include <DB/search/ImageSearchInfo.h>
#include <kpabase/FileNameList.h>

#include <QObject>
#include <QPointer>
#include <memory>

class QProgressBar;

namespace DB
{
class FileWriter;
}
namespace DB
{
Q_NAMESPACE

class CategoryCollection;
class Category;
class MD5Map;
class MemberMap;
class FileName;
class TagInfo;
class UIDelegate;

/**
 * @brief The SearchOption enum influences the way a search is conducted.
 */
enum class SearchOption {
    NoOption = 0b0000, ///< No special query options
    RequireOnDisk = 0b0001, ///< Check for each file, if it is currently available on disk and discard any image that is not available.
    AllowRangeMatch = 0b0010 ///< In addition to the standard date match (returning images within the given range), allow matching on image dates that overlap with the search range ("range match").
};
Q_DECLARE_FLAGS(SearchOptions, SearchOption);
Q_FLAG_NS(SearchOption);

/**
 * @brief The ClassificationMode enum can be used to short-circuit classification in the classify() method.
 * This allows you to only check whether a given category has more than one sub-category (including the "No other" category).
 * In other words, you can use a partial count when all you want to know is whether further search refinement is possible
 * in a category.
 * @see ImageDB::classify()
 * @see Browser::OverviewPage::updateImageCount()
 */
enum class ClassificationMode {
    FullCount ///< @brief run a full classification. This is normally what you want.
    ,
    PartialCount ///< @brief Count until at least 2 categories are found
};

class ImageDB : public QObject
{
    Q_OBJECT

public:
    static ImageDB *instance();
    static void setupXMLDB(const QString &configFile, UIDelegate &delegate);
    static void deleteInstance();
    static QString NONE();

    UIDelegate &uiDelegate() const;
    DB::FileNameList currentScope(bool requireOnDisk) const;

    DB::FileName findFirstItemInRange(const FileNameList &files, const ImageDate &range, bool includeRanges) const;

    bool untaggedCategoryFeatureConfigured() const;

    int totalCount() const;
    DB::ImageInfoList search(const ImageSearchInfo &, bool requireOnDisk) const;
    DB::ImageInfoList search(const DB::ImageSearchInfo &searchInfo, DB::SearchOptions options = DB::SearchOption::NoOption) const;

    /**
     * @brief Rename category in media items stored in database.
     */
    void renameCategory(const QString &oldName, const QString newName);

    /**
     * @brief classify computes a histogram of tags within a category.
     * I.e. for each sub-category within a given category it counts all images matching the current context, and
     * computes the date range for those images.
     *
     * @param info ImageSearchInfo describing the current search context
     * @param category the category for which images should be classified
     * @param typemask images/videos/both
     * @param mode whether accurate counts are required or not
     * @return a mapping of sub-category (tags/tag-groups) to the number of images (and the associated date range)
     */
    QMap<QString, CountWithRange> classify(const ImageSearchInfo &info, const QString &category, MediaType typemask, ClassificationMode mode = ClassificationMode::FullCount);
    FileNameList files(MediaType type = anyMediaType) const;
    ImageInfoList images() const;
    /**
     * @brief addImages to the database.
     * The parameter \p doUpdate decides whether all bookkeeping should be done right away
     * (\c true; the "normal" use-case), or if it should be deferred until later(\c false).
     * If doUpdate is deferred, either commitDelayedImages() or clearDelayedImages() needs to be called afterwards.
     * @param images
     * @param doUpdate
     */
    void addImages(const ImageInfoList &images, bool doUpdate = true);
    void commitDelayedImages();
    void clearDelayedImages();
    /** @short Update file name stored in the DB */
    void renameImage(const ImageInfoPtr info, const DB::FileName &newName);

    void addToBlockList(const DB::FileNameList &list);
    bool isBlocking(const DB::FileName &fileName);
    void deleteList(const DB::FileNameList &list);
    ImageInfoPtr info(const DB::FileName &fileName) const;
    MemberMap &memberMap();
    void save(const QString &fileName, bool isAutoSave);
    MD5Map *md5Map();
    void sortAndMergeBackIn(const DB::FileNameList &fileNameList);

    CategoryCollection *categoryCollection();
    const CategoryCollection *categoryCollection() const;

    /**
     * Reorder the items in the database by placing all the items given in
     * selection directly before or after the given item.
     * If the parameter "after" determines where to place it.
     */
    void reorder(const DB::FileName &item, const DB::FileNameList &selection, bool after);

    /** @short Create a stack of images/videos/whatever
     *
     * If the specified images already belong to different stacks, then no
     * change happens and the function returns false.
     *
     * If some of them are in one stack and others aren't stacked at all, then
     * the unstacked will be added to the existing stack and we return true.
     *
     * If none of them are stacked, then a new stack is created and we return
     * true.
     *
     * All images which previously weren't in the stack are added in order they
     * are present in the provided list and after all items that are already in
     * the stack. The order of images which were already in the stack is not
     * changed.
     * */
    bool stack(const DB::FileNameList &items);

    /** @short Remove all images from whichever stacks they might be in
     *
     * We might destroy some stacks in the process if they become empty or just
     * containing one image.
     *
     * This function doesn't touch the order of images at all.
     * */
    void unstack(const DB::FileNameList &items);

    /** @short Return a list of images which are in the same stack as the one specified.
     *
     * Returns an empty list when the image is not stacked.
     *
     * They are returned sorted according to their stackOrder.
     * */
    DB::FileNameList getStackFor(const DB::FileName &referenceImage) const;

    void copyData(const DB::FileName &from, const DB::FileName &to);

    Exif::Database *exifDB() const;

    const DB::TagInfo *untaggedTag() const;

    static int fileVersion();
    static DB::ImageInfoPtr createImageInfo(const DB::FileName &fileName, DB::ReaderPtr, ImageDB *db = nullptr, const QMap<QString, QString> *newToOldCategory = nullptr);
    static void possibleLoadCompressedCategories(DB::ReaderPtr reader, DB::ImageInfoPtr info, ImageDB *db, const QMap<QString, QString> *newToOldCategory = nullptr);
public Q_SLOTS:
    void setDateRange(const ImageDate &, bool includeFuzzyCounts);
    void clearDateRange();
    virtual MediaCount count(const ImageSearchInfo &info);
    virtual void slotReread(const DB::FileNameList &list, DB::ExifMode mode);
    void setCurrentScope(const DB::ImageSearchInfo &info);

Q_SIGNALS:
    void totalChanged(int);
    void dirty();
    void imagesDeleted(const DB::FileNameList &);

protected:
    ImageDB(const QString &configFile, UIDelegate &delegate);

    bool rangeInclude(DB::ImageInfoPtr info) const;

    DB::ImageInfoList takeImagesFromSelection(const DB::FileNameList &selection);
    void insertList(const DB::FileName &fileName, const DB::ImageInfoList &list, bool after);
    static void readOptions(DB::ImageInfoPtr info, DB::ReaderPtr reader, const QMap<QString, QString> *newToOldCategory = nullptr);

    ImageInfoList m_clipboard;
    UIDelegate &m_UI;
    std::unique_ptr<Exif::Database> m_exifDB;
    ImageDate m_selectionRange;
    bool m_includeFuzzyCounts;

protected Q_SLOTS:
    void renameItem(DB::Category *category, const QString &oldName, const QString &newName);
    void deleteItem(DB::Category *category, const QString &value);
    void lockDB(bool lock, bool exclude);
    void markDirty();
    /**
     * @brief setUntaggedTag sets the untaggedTag for the database and also updates the corresponding settings value.
     * @param tag
     * @see Settings::SettingsData::untaggedTag()
     * @see Settings::SettingsData::untaggedCategory()
     */
    void setUntaggedTag(DB::TagInfo *tag);
    void setUntaggedTag(const QString &category, const QString &tag);

private:
    friend class DB::FileReader;
    friend class DB::FileWriter;

    static void connectSlots();
    static ImageDB *s_instance;
    DB::ImageSearchInfo m_currentScope;
    QPointer<DB::TagInfo> m_untaggedTag;

    void forceUpdate(const DB::ImageInfoList &images);

    QString m_fileName;
    DB::ImageInfoList m_images;
    QSet<DB::FileName> m_blockList;
    DB::ImageInfoList m_missingTimes;
    DB::CategoryCollection m_categoryCollection;
    DB::MemberMap m_members;
    DB::MD5Map m_md5map;
    // QMap<QString, QString> m_settings;

    DB::StackID m_nextStackId;
    typedef QMap<DB::StackID, DB::FileNameList> StackMap;
    mutable StackMap m_stackMap;
    DB::ImageInfoList m_delayedUpdate;
    mutable QHash<const QString, DB::ImageInfoPtr> m_imageCache;
    mutable QHash<const QString, DB::ImageInfoPtr> m_delayedCache;

    // used for checking if any images are without image attribute from the database.
    static bool s_anyImageWithEmptySize;
};
}
#endif /* IMAGEDB_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
