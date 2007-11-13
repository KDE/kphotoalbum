/*
  Copyright (C) 2006-2007 Tuomas Suutari <thsuut@utu.fi>

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

#include <qstringlist.h>
#include <qpair.h>
#include <QList>
#include "DatabaseConnection.h"
#include "DB/Category.h"
#include "DB/ImageInfo.h"
#include "DB/ImageInfoPtr.h"
namespace DB {
    class ImageDate;
    class ImageSearchInfo;
    class OptionSimpleMatcher;
}

namespace SQLDB
{

using Utilities::StringSet;

typedef QList<DB::OptionSimpleMatcher*> MatcherList;
typedef QList<MatcherList> MatcherListList;


class QueryHelper: public DatabaseConnection
{
public:
    typedef QList<QVariant> Bindings;

    explicit QueryHelper(const DatabaseConnection& connection):
        DatabaseConnection(connection)
    {
    }

    uint mediaItemCount(DB::MediaType typemask=DB::anyMediaType,
                        QList<int>* scope=0) const;
    QList<int> mediaItemIds(DB::MediaType typemask) const;
    QStringList filenames() const;

    int mediaItemId(const QString& filename) const;
    QList< QPair<int, QString> > mediaItemIdFileMap() const;
    QString mediaItemFilename(int id) const;

    QStringList categoryNames() const;

    int categoryId(const QString& category) const;

    QList<int> tagIdsOfCategory(const QString& category) const;
    QStringList tagNamesOfCategory(int categoryId) const;

    QStringList folders() const;

    QList<int> tagIdList(const QString& category,
                              const QString& item) const;

    StringStringList
    memberGroupConfiguration(const QString& category) const;

    QMap<int, StringSet >
    mediaIdTagsMap(const QString& category, DB::MediaType typemask) const;

    void getMediaItem(int id, DB::ImageInfo& info) const;
    void insertMediaItemsLast(const QList<DB::ImageInfoPtr>& items);
    void updateMediaItem(int id, const DB::ImageInfo& info);
    void removeMediaItem(const QString& filename);

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

    bool isBlocked(const QString& filename) const;
    void addBlockItems(const QStringList& filenames);

    bool containsMD5Sum(const DB::MD5& md5sum) const;
    QString filenameForMD5Sum(const DB::MD5& md5sum) const;

    void sortMediaItems(const QStringList& filenames);
    void moveMediaItems(const QStringList& filenames,
                        const QString& destinationFilename, bool after);

    QList<int>
    searchMediaItems(const DB::ImageSearchInfo& search,
                     DB::MediaType typemask=DB::anyMediaType) const;

    QString findFirstFileInTimeRange(const DB::ImageDate& range,
                                     bool includeRanges) const;
    QString findFirstFileInTimeRange(const DB::ImageDate& range,
                                     bool includeRanges,
                                     const QList<int>& idList) const;
    QMap<QString, uint> classify(const QString& category,
                                 DB::MediaType typemask=DB::anyMediaType,
                                 QList<int>* scope=0) const;

protected:
    Bindings imageInfoToBindings(const DB::ImageInfo& info);

    QList<int> mediaItemIdsForFilenames(const QStringList& filenames) const;
    int insertTag(int categoryId, const QString& name);
    int insertDir(const QString& dirname);
    void insertMediaItem(const DB::ImageInfo& info, int place=0);
    void insertMediaTag(int mediaId, int tagId);
    void insertMediaItemTags(int mediaId, const DB::ImageInfo& info);
    void insertMediaItemDrawings(int mediaId, const DB::ImageInfo& info);
    QList<int> directMembers(int tagId) const;
    void addBlockItem(const QString& filename);
    int mediaPlaceByFilename(const QString& filename) const;
    void makeMediaPlacesContinuous();
    QList<int>
    getMatchingFiles(MatcherList matches,
                     DB::MediaType typemask=DB::anyMediaType) const;
    QString findFirstFileInTimeRange(const DB::ImageDate& range,
                                     bool includeRanges,
                                     const QList<int>* idList) const;
};

}

#endif /* QUERYHELPER_H */
