/*
  Copyright (C) 2006 Tuomas Suutari <thsuut@utu.fi>

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
#include <kexidb/connection.h>
#include <kexidb/driver.h>
#include <kexidb/cursor.h>
#include "DB/Category.h"
#include "DB/ImageInfo.h"
#include "DB/ImageInfoPtr.h"
#include "Cursor.h"
namespace DB {
    class ImageDate;
    class ImageSearchInfo;
    class OptionSimpleMatcher;
}

namespace SQLDB
{

typedef QValueList<DB::OptionSimpleMatcher*> MatcherList;
typedef QValueList<MatcherList> MatcherListList;


/** Copy some list to QValueList of QVariants.
 *
 * Class T should support iterating interface (e.g. const_iterator,
 * begin(), end()) and should be convertable to QVariant.
 *
 * @param l the list to copy from
 * @return list which contains elements of l in same order, but as QVariants
 */
template <class T>
QValueList<QVariant> toVariantList(const T& l)
{
    QValueList<QVariant> r;
    for (typename T::const_iterator i = l.begin(); i != l.end(); ++i)
        r << *i;
    return r;
}

class QueryHelper
{
public:
    typedef QValueList<QVariant> Bindings;
    class Result
    {
    public:
        Result(KexiDB::Cursor* cursor);
        QStringList asStringList();
        QValueList<QString[2]> asString2List();
        QValueList<QString[3]> asString3List();
        QValueList<int> asIntegerList();
        QValueList< QPair<int, QString> > asIntegerStringPairs();
        QVariant firstItem();
        RowData getRow(uint n=0);
        Cursor cursor();

    private:
        Cursor _cursor;
    };

    static void setup(KexiDB::Connection& connection);
    static QueryHelper* instance();

    void executeStatement(const QString& statement,
                          const Bindings& bindings=Bindings());
    Result executeQuery(const QString& query,
                        const Bindings& bindings=Bindings()) const;

    uint mediaItemCount(int typemask=DB::anyMediaType,
                        QValueList<int>* scope=0);
    QValueList<int> allMediaItemIdsByType(int typemask);
    QStringList relativeFilenames();

    int idForFilename(const QString& relativePath);
    QString filenameForId(int id, bool fullPath=false);

    QStringList categoryNames() const;

    int idForCategory(const QString& category);
    QString categoryForId(int id);

    QValueList<int> tagIdsOfCategory(const QString& category);
    QStringList tagNamesOfCategory(int categoryId);

    QStringList folders();

    QValueList<int> idListForTag(const QString& category, const QString& item);

    QValueList<QString[3]> memberGroupConfiguration() const;

    QValueList< QPair<int, QString> > mediaIdTagPairs(const QString& category,
                                                      int typemask);

    void getMediaItem(int id, DB::ImageInfo& info);
    void insertMediaItemsLast(const QValueList<DB::ImageInfoPtr>& items);
    void updateMediaItem(int id, const DB::ImageInfo& info);
    void removeMediaItem(const QString& relativePath);

    void insertCategory(const QString& name, const QString& icon, bool visible,
                        DB::Category::ViewType type,
                        DB::Category::ViewSize size);
    void removeCategory(const QString& name);
    void changeCategoryName(int id, const QString& newName);
    void changeCategoryIcon(int id, const QString& icon);
    void changeCategoryVisible(int id, bool visible);
    void changeCategoryViewType(int id, DB::Category::ViewType type);
    void changeCategoryViewSize(int id, DB::Category::ViewSize size);

    void insertTagFirst(int categoryId, const QString& name);
    void removeTag(int categoryId, const QString& name);

    bool isBlocked(const QString& relativePath);
    void addBlockItems(const QStringList& relativePaths);

    bool containsMD5Sum(const QString& md5sum);
    QString filenameForMD5Sum(const QString& md5sum);

    void sortMediaItems(const QStringList& relativePaths);
    void moveMediaItems(const QStringList& sourceItems,
                        const QString& destination, bool after);

    QValueList<int> searchMediaItems(const DB::ImageSearchInfo& search,
                                     int typemask=DB::anyMediaType);

    QString findFirstFileInTimeRange(const DB::ImageDate& range,
                                     bool includeRanges);
    QString findFirstFileInTimeRange(const DB::ImageDate& range,
                                     bool includeRanges,
                                     const QValueList<int>& idList);

protected:
    KexiDB::Connection *_connection;
    KexiDB::Driver *_driver;

    QueryHelper(KexiDB::Connection& connection);

    QString sqlRepresentation(const QVariant& x) const;
    void bindValues(QString &s, const Bindings& b) const;
    Q_ULLONG insert(const QString& tableName, const QString& aiFieldName,
                    const QStringList& fields, const Bindings& values);

    Bindings imageInfoToBindings(const DB::ImageInfo& info);

    QValueList<int> idsForFilenames(const QStringList& relativePaths);
    int idForTag(const QString& category, const QString& item);
    int insertTag(int categoryId, const QString& name);
    int insertDir(const QString& relativePath);
    void insertMediaItem(const DB::ImageInfo& info, int place=0);
    void insertMediaTag(int mediaId, int tagId);
    void insertMediaItemTags(int mediaId, const DB::ImageInfo& info);
    void insertMediaItemDrawings(int mediaId, const DB::ImageInfo& info);
    QValueList<int> directMembers(int tagId);
    void addBlockItem(const QString& relativePath);
    int mediaPlaceByFilename(const QString& relativePath);
    void makeMediaPlacesContinuous();
    QValueList<int> getMatchingFiles(MatcherList matches,
                                     int typemask=DB::anyMediaType);
    QString findFirstFileInTimeRange(const DB::ImageDate& range,
                                     bool includeRanges,
                                     const QValueList<int>* idList);

private:
    static QueryHelper* _instance;

    // Copying is not allowed
    QueryHelper(const QueryHelper&);
    QueryHelper& operator=(const QueryHelper&);
};

}

#endif /* QUERYHELPER_H */
