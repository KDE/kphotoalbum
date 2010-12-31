/*
  Copyright (C) 2006-2010 Tuomas Suutari <thsuut@utu.fi>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program (see the file COPYING); if not, write to the
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
  MA 02110-1301 USA.
*/

#ifndef QUERYHELPER_H
#define QUERYHELPER_H

#include "QSqlConnection.h"
#include "DB/Category.h"
#include "DB/ImageInfo.h"
#include "DB/ImageInfoPtr.h"
#include "DB/RawId.h"
#include "DB/Id.h"
#include <qstringlist.h>
#include <qpair.h>
namespace DB {
    class ImageDate;
    class ImageSearchInfo;
class SimpleCategoryMatcher;
}

namespace SQLDB
{

using Utilities::StringSet;

typedef QList<DB::SimpleCategoryMatcher*> MatcherList;
typedef QList<MatcherList> MatcherListList;

class QueryHelper: public QSqlConnection
{
public:
    typedef QList<QVariant> Bindings;

    // TODO: Reorganize whole QueryHelper to separate helper functions.
    // This is just a trick to avoid changing QueryHelper.cpp at this point.
    explicit QueryHelper(const Connection& connection):
        QSqlConnection(*static_cast<const QSqlConnection*>(&connection))
    {
    }

    uint mediaItemCount(DB::MediaType typemask=DB::anyMediaType,
                        QList<DB::RawId>* scope=0) const;
    QList<DB::RawId> mediaItemIds(DB::MediaType typemask) const;
    QStringList filenames() const;

    DB::RawId mediaItemId(const QString& filename) const;
    QList< QPair<DB::RawId, QString> > mediaItemIdFileMap() const;
    QString mediaItemFilename(DB::RawId id) const;

    QStringList categoryNames() const;

    int categoryId(const QString& category) const;

    QList<int> tagIdsOfCategory(const QString& category) const;
    QStringList tagNamesOfCategory(int categoryId) const;

    QStringList folders() const;

    QList<int> tagIdList(const QString& category,
                              const QString& item) const;

    StringStringList
    memberGroupConfiguration(const QString& category) const;

    QMap<DB::RawId, StringSet>
    mediaIdTagsMap(const QString& category, DB::MediaType typemask) const;

    void getMediaItem(DB::RawId id, DB::ImageInfo& info) const;
    void insertMediaItemsLast(const QList<DB::ImageInfoPtr>& items);
    void updateMediaItem(DB::RawId id, const DB::ImageInfo& info);
    void removeMediaItem(DB::RawId id);

    void insertCategory(const QString& name, const QString& icon, bool visible,
                        DB::Category::ViewType type,
                        int thumbnailSize);
    void removeCategory(const QString& name);
    QString categoryName(int id) const;
    QString categoryIcon(int id) const;
    bool categoryVisible(int id) const;
    DB::Category::ViewType categoryViewType(int id) const;
    void changeCategoryName(int id, const QString& newName);
    void changeCategoryIcon(int id, const QString& icon);
    void changeCategoryVisible(int id, bool visible);
    void changeCategoryViewType(int id, DB::Category::ViewType type);

    int tagId(const QString& category, const QString& item) const;
    void insertTagFirst(int categoryId, const QString& name);
    void removeTag(int categoryId, const QString& name);

    bool isIgnored(const QString& filename) const;
    void addIgnoredFiles(const QStringList& filenames);

    bool containsMD5Sum(const DB::MD5& md5sum) const;
    QString filenameForMD5Sum(const DB::MD5& md5sum) const;

    void sortFiles(const QList<DB::RawId>& files);
    void moveMediaItems(
        const QList<DB::RawId>& files,
        DB::RawId destinationFile,
        bool after);

    QList<DB::RawId>
    searchMediaItems(const DB::ImageSearchInfo& search,
                     DB::MediaType typemask=DB::anyMediaType) const;

    DB::Id findFirstFileInTimeRange(
        const DB::ImageDate& range,
        bool includeRanges) const;
    DB::Id findFirstFileInTimeRange(
        const DB::ImageDate& range,
        bool includeRanges,
        const QList<DB::RawId>& idList) const;
    QMap<QString, uint> classify(const QString& category,
                                 DB::MediaType typemask=DB::anyMediaType,
                                 QList<DB::RawId>* scope=0) const;
    /** Update the file table so that given files will go to same stack.
     *
     * \note The caller has responsibility to update the file infos.
     *
     * \return The id of the stack those files are put into
     *
     * \throws OperationNotPossible if stacking cannot be done because
     * some of the given files already belong to different stacks,
     * also then nothing is changed in the database
     */
    DB::StackID stackFiles(QList<DB::RawId> files);

    /** Set stack_id of given files to NULL in the file table.
     */
    void unstackFiles(QList<DB::RawId> files);

    /** Get list of files that have the same stackid as the given file.
     */
    QList<DB::RawId> getStackOfFile(DB::RawId referenceFile) const;

    /** Get infos of files with given ids.
     *
     * \return map from file id to info pointer
     */
    QMap<DB::RawId, DB::ImageInfoPtr> getInfosOfFiles(const QList<DB::RawId>& idList) const;

protected:
    Bindings imageInfoToBindings(const DB::ImageInfo& info);

    QList<DB::RawId> mediaItemIdsForFilenames(const QStringList& filenames) const;
    int insertTag(int categoryId, const QString& name);
    int insertDir(const QString& dirname);
    void insertMediaItem(const DB::ImageInfo& info, int position=0);
    void insertMediaTag(DB::RawId mediaId, int tagId);
    void insertMediaItemTags(DB::RawId mediaId, const DB::ImageInfo& info);
    QList<int> directMembers(int tagId) const;
    void addIgnoredFile(const QString& filename);
    int getPositionOfFile(DB::RawId fileId) const;
    void makeMediaPositionsContinuous();
    QList<DB::RawId>
    getMatchingFiles(MatcherList matches,
                     DB::MediaType typemask=DB::anyMediaType) const;
    DB::Id findFirstFileInTimeRange(
        const DB::ImageDate& range,
        bool includeRanges,
        const QList<DB::RawId>* idList) const;
};

}

#endif /* QUERYHELPER_H */
