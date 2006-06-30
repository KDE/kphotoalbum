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
#include <kexidb/connection.h>
#include <kexidb/driver.h>
#include <kexidb/cursor.h>
#include <DB/ImageInfo.h>

namespace SQLDB
{

class QueryHelper
{
public:
    typedef QValueList<QVariant> Bindings;
    class Result
    {
    public:
        Result(KexiDB::Cursor* cursor, KexiDB::Connection* connection);
        ~Result();
        bool destroy();
        QStringList asStringList();
        QValueList<QString[3]> asString3List();
        QValueList<int> asIntegerList();
        QVariant firstItem();
        KexiDB::Cursor* cursor(); // caller frees

    private:
        KexiDB::Cursor* _cursor;
        KexiDB::Connection* _connection;
    };

    static void setup(KexiDB::Connection* connection);
    static QueryHelper* instance();

    bool executeStatement(const QString& statement,
                          Bindings bindings=Bindings());
    Result executeQuery(const QString& query,
                        Bindings bindings=Bindings());
    Q_ULLONG insert(QString tableName, QString aiFieldName,
                    QStringList fields, Bindings values);

    QString filenameForId(int id, bool fullPath=false);
    int idForFilename(const QString& relativePath);
    QString categoryForId(int id);
    int idForCategory(const QString& category);
    QValueList<int> tagIdsOfCategory(const QString& category);
    QStringList membersOfCategory(int categoryId);
    QStringList membersOfCategory(const QString& category);
    QStringList folders();
    QValueList<int> allMediaItemIds();
    QValueList<int> allMediaItemIdsOfType(DB::MediaType type);
    int insertTag(int categoryId, QString name);
    void insertMediaTag(int mediaId, int tagId);
    int insertDir(QString path);
    bool getMediaItem(int id, DB::ImageInfo& info);
    void insertMediaItemTags(int mediaId, const DB::ImageInfo& info);
    void insertMediaItem(const DB::ImageInfo& info);
    void updateMediaItem(int id, const DB::ImageInfo& info);
    QValueList<int> getDirectMembers(int tagId);
    int idForTag(QString category, QString item);
    QValueList<int> idListForTag(QString category, QString item);
    void addBlockItem(const QString& filename);
    void addBlockItems(const QStringList& filenames);
    bool isBlocked(const QString& filename);

protected:
    KexiDB::Connection *_connection;
    KexiDB::Driver *_driver;

    QueryHelper(KexiDB::Connection* connection);
    QString getSQLRepresentation(const QVariant& x);
    void bindValues(QString &s, const Bindings& b);
    KexiDB::Cursor* runQuery(const QString& query);
    void showLastError();

private:
    static QueryHelper* _instance;

    // Copying is not allowed
    QueryHelper(const QueryHelper&);
    QueryHelper& operator=(const QueryHelper&);
};

}

#endif /* QUERYHELPER_H */
