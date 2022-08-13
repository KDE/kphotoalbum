// SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef XMLDB_DATABASE_H
#define XMLDB_DATABASE_H

#include "FileReader.h"
#include "XMLCategoryCollection.h"

#include <DB/Category.h>
#include <DB/CategoryCollection.h>
#include <DB/ImageDB.h>
#include <DB/ImageInfoList.h>
#include <DB/ImageSearchInfo.h>
#include <DB/MD5Map.h>
#include <DB/MemberMap.h>
#include <kpabase/FileNameList.h>

#include <qdom.h>
#include <qstringlist.h>

namespace DB
{
class ImageInfo;
}

namespace XMLDB
{
class Database : public DB::ImageDB
{
    Q_OBJECT

public:
    uint totalCount() const override;
    DB::ImageInfoList search(
        const DB::ImageSearchInfo &,
        bool requireOnDisk = false) const override;
    void renameCategory(const QString &oldName, const QString newName) override;

    QMap<QString, DB::CountWithRange> classify(const DB::ImageSearchInfo &info, const QString &category, DB::MediaType typemask, DB::ClassificationMode mode) override;
    DB::FileNameList files(DB::MediaType type) const override;
    DB::ImageInfoList images() const override;
    void addImages(const DB::ImageInfoList &files, bool doUpdate) override;
    void commitDelayedImages() override;
    void clearDelayedImages() override;
    void renameImage(DB::ImageInfoPtr info, const DB::FileName &newName) override;

    void addToBlockList(const DB::FileNameList &list) override;
    bool isBlocking(const DB::FileName &fileName) override;
    void deleteList(const DB::FileNameList &list) override;
    DB::ImageInfoPtr info(const DB::FileName &fileName) const override;
    DB::MemberMap &memberMap() override;
    void save(const QString &fileName, bool isAutoSave) override;
    DB::MD5Map *md5Map() override;
    void sortAndMergeBackIn(const DB::FileNameList &idList) override;
    DB::CategoryCollection *categoryCollection() override;
    const DB::CategoryCollection *categoryCollection() const override;
    QExplicitlySharedDataPointer<DB::ImageDateCollection> rangeCollection() override;
    void reorder(
        const DB::FileName &item,
        const DB::FileNameList &cutList,
        bool after) override;

    static DB::ImageInfoPtr createImageInfo(const DB::FileName &fileName, ReaderPtr, Database *db = nullptr, const QMap<QString, QString> *newToOldCategory = nullptr);
    static void possibleLoadCompressedCategories(ReaderPtr reader, DB::ImageInfoPtr info, Database *db, const QMap<QString, QString> *newToOldCategory = nullptr);
    bool stack(const DB::FileNameList &items) override;
    void unstack(const DB::FileNameList &files) override;
    DB::FileNameList getStackFor(const DB::FileName &referenceId) const override;
    void copyData(const DB::FileName &from, const DB::FileName &to) override;

    static int fileVersion();

protected:
    DB::ImageInfoList searchPrivate(
        const DB::ImageSearchInfo &,
        bool requireOnDisk,
        bool onlyItemsMatchingRange) const;
    bool rangeInclude(DB::ImageInfoPtr info) const;

    DB::ImageInfoList takeImagesFromSelection(const DB::FileNameList &list);
    void insertList(const DB::FileName &id, const DB::ImageInfoList &list, bool after);
    static void readOptions(DB::ImageInfoPtr info, ReaderPtr reader, const QMap<QString, QString> *newToOldCategory = nullptr);

protected slots:
    void renameItem(DB::Category *category, const QString &oldName, const QString &newName);
    void deleteItem(DB::Category *category, const QString &option);
    void lockDB(bool lock, bool exclude) override;

private:
    friend class DB::ImageDB;
    friend class FileReader;
    friend class FileWriter;

    Database(const QString &configFile, DB::UIDelegate &delegate);
    void forceUpdate(const DB::ImageInfoList &);

    QString m_fileName;
    DB::ImageInfoList m_images;
    QSet<DB::FileName> m_blockList;
    DB::ImageInfoList m_missingTimes;
    XMLCategoryCollection m_categoryCollection;
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

#endif /* XMLDB_DATABASE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
