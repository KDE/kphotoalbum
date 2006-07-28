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
#include "DB/ImageInfo.h"
#include "DB/ImageInfoPtr.h"
#include "Cursor.h"

namespace SQLDB
{

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
        ~Result();
        QStringList asStringList();
        QValueList<QString[2]> asString2List();
        QValueList<QString[3]> asString3List();
        QValueList<int> asIntegerList();
        QValueList< QPair<int, QString> > asIntegerStringPairs();
        QVariant firstItem();
        RowData getRow(uint n=0);
        Cursor cursor();

    private:
        KexiDB::Cursor* _cursor;
    };

    static void setup(KexiDB::Connection& connection);
    static QueryHelper* instance();

    void executeStatement(const QString& statement,
                          const Bindings& bindings=Bindings());
    Result executeQuery(const QString& query,
                        const Bindings& bindings=Bindings());

    QStringList relativeFilenames();
    QString filenameForId(int id, bool fullPath=false);
    int idForFilename(const QString& relativePath);
    QString categoryForId(int id);
    int idForCategory(const QString& category);
    QValueList<int> tagIdsOfCategory(const QString& category);
    QStringList membersOfCategory(int categoryId);
    QStringList membersOfCategory(const QString& category);
    QStringList folders();
    QValueList<int> allMediaItemIds();
    QValueList<int> allMediaItemIdsByType(int typemask);
    int insertTag(int categoryId, const QString& name);
    void insertTagFirst(int categoryId, const QString& name);
    void removeTag(int categoryId, const QString& name);
    void insertMediaTag(int mediaId, int tagId);
    int insertDir(const QString& relativePath);
    void getMediaItem(int id, DB::ImageInfo& info);
    void insertMediaItemTags(int mediaId, const DB::ImageInfo& info);
    void insertMediaItemDrawings(int mediaId, const DB::ImageInfo& info);
    void insertMediaItem(const DB::ImageInfo& info, int place=0);
    void insertMediaItemsLast(const QValueList<DB::ImageInfoPtr>& items);
    void updateMediaItem(int id, const DB::ImageInfo& info);
    QValueList<int> getDirectMembers(int tagId);
    int idForTag(const QString& category, const QString& item);
    QValueList<int> idListForTag(const QString& category, const QString& item);
    void addBlockItem(const QString& relativePath);
    void addBlockItems(const QStringList& relativePaths);
    bool isBlocked(const QString& relativePath);
    void removeMediaItem(const QString& relativePath);
    bool containsMD5Sum(const QString& md5sum);
    QString filenameForMD5Sum(const QString& md5sum);
    QValueList< QPair<int, QString> >
    getMediaIdTagPairs(const QString& category, int typemask);
    void moveMediaItems(const QStringList& sourceItems,
                        const QString& destination, bool after);
    int mediaPlaceByFilename(const QString& relativePath);

protected:
    KexiDB::Connection *_connection;
    KexiDB::Driver *_driver;

    QueryHelper(KexiDB::Connection& connection);

    QString getSQLRepresentation(const QVariant& x);
    void bindValues(QString &s, const Bindings& b);
    Q_ULLONG insert(const QString& tableName, const QString& aiFieldName,
                    const QStringList& fields, const Bindings& values);
    Bindings imageInfoToBindings(const DB::ImageInfo& info);

private:
    static QueryHelper* _instance;

    // Copying is not allowed
    QueryHelper(const QueryHelper&);
    QueryHelper& operator=(const QueryHelper&);
};

}

#endif /* QUERYHELPER_H */
