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

#include "config.h" // HASKEXIDB

#ifdef HASKEXIDB

#include "QueryHelper.h"
#include "KexiHelpers.h"
#include "Settings/SettingsData.h"
#include <klocale.h>
#include <qsize.h>

using namespace SQLDB;

QueryHelper::Result::Result(KexiDB::Cursor* cursor,
                            KexiDB::Connection* connection):
    _cursor(cursor),
    _connection(connection)
{
}

QueryHelper::Result::~Result()
{
    destroy();
}

KexiDB::Cursor* QueryHelper::Result::cursor()
{
    KexiDB::Cursor* tmp = _cursor;
    _cursor = 0;
    return tmp;
}

bool QueryHelper::Result::destroy()
{
    if (_cursor) {
        _connection->deleteCursor(_cursor);
        _cursor = 0;
        return true;
    }
    return false;
}

QStringList QueryHelper::Result::asStringList()
{
    QStringList r;
    if (_cursor) {
        r = readStringsFromCursor(*_cursor);
        destroy();
    }
    return r;
}

QValueList<QString[3]> QueryHelper::Result::asString3List()
{
    QValueList<QString[3]> r;
    if (_cursor) {
        r = readString3sFromCursor(*_cursor);
        destroy();
    }
    return r;
}

QValueList<int> QueryHelper::Result::asIntegerList()
{
    QValueList<int> r;
    if (_cursor) {
        r = readIntsFromCursor(*_cursor);
        destroy();
    }
    return r;
}

QVariant QueryHelper::Result::firstItem()
{
    QVariant r;
    if (_cursor) {
        _cursor->moveFirst();
        if (!_cursor->eof())
             r = _cursor->value(0);
        destroy();
    }
    return r;
}

QueryHelper* QueryHelper::_instance = 0;

QueryHelper::QueryHelper(KexiDB::Connection* connection):
    _connection(connection),
    _driver(0)
{
    if (_connection) {
        _driver = _connection->driver();
    }
}

void QueryHelper::setup(KexiDB::Connection* connection)
{
    if (_instance)
        delete _instance;
    _instance = new QueryHelper(connection);
}

QueryHelper* QueryHelper::instance()
{
    if (!_instance)
        // TODO: error handling
        qFatal("QueryHelper::instance() called before setup");
    return _instance;
}

void QueryHelper::bindValues(QString &s, const Bindings& b)
{
    int n = 0;
    for (Bindings::const_iterator i = b.begin(); i != b.end(); ++i) {
        do {
            n = s.find("%s", n);
        } while (n >= 1 && s.at(n - 1) == '%');
        if (n == -1)
            break;
        s = s.replace(n, 2, _driver->valueToSQL(fieldTypeFor(*i), *i));
    }
}

KexiDB::Cursor* QueryHelper::runQuery(const QString& query)
{
    KexiDB::Cursor* c;
    c = _connection->executeQuery(query);
    if (!c) {
        showLastError();
    }
    return c;
}

void QueryHelper::showLastError()
{
    if (!_connection->error())
        return;

    QString msg =
        i18n("Error running query: %1\nError was: %2")
        .arg(_connection->recentSQLString())
        .arg(_connection->errorMsg());
    // TODO: better handling of errors
    qFatal("%s", msg.local8Bit().data());
}

bool QueryHelper::executeStatement(const QString& statement,
                                   Bindings bindings)
{
    QString s = statement;
    /*
    for (Bindings::const_iterator i = bindings.begin();
         i != bindings.end(); ++i) {
        s = s.arg(_driver->valueToSQL(fieldTypeFor(*i), *i));
    }
    */
    bindValues(s, bindings);

    //TODO: remove debug
    qDebug("Executing statement: %s", s.local8Bit().data());

    return _connection->executeSQL(s);
}

QueryHelper::Result QueryHelper::executeQuery(const QString& query,
                                              Bindings bindings)
{
    QString q = query;
    /*
    for (Bindings::const_iterator i = bindings.begin();
         i != bindings.end(); ++i) {
        q = q.arg(_driver->valueToSQL(fieldTypeFor(*i), *i));
    }
    */
    bindValues(q, bindings);

    //TODO: remove debug
    qDebug("Executing query: %s", q.local8Bit().data());

    return Result(runQuery(q), _connection);
}

Q_ULLONG QueryHelper::insert(QString tableName, QString aiFieldName,
                             QStringList fields, Bindings values)
{
    Q_ASSERT(fields.count() == values.count());

    QString q = "INSERT INTO %1(%2) VALUES (%3)";
    q = q.arg(tableName);
    q = q.arg(fields.join(", "));
    QStringList l;
    for (Bindings::size_type i = 0; i < values.count(); ++i)
        l.append("%s");
    q = q.arg(l.join(", "));
    if (executeStatement(q, values))
        return _connection->lastInsertedAutoIncValue(aiFieldName, tableName);
    else
        return 0;
}

QString QueryHelper::filenameForId(int id, bool fullPath)
{
    QString fn = executeQuery("SELECT filename FROM media WHERE id=%s",
                              Bindings() << id).firstItem().toString();
    if (fullPath)
        return Settings::SettingsData::instance()->imageDirectory() + fn;
    else
        return fn;
}

int QueryHelper::idForFilename(const QString& relativePath)
{
    return executeQuery("SELECT id FROM media WHERE filename=%s",
                        Bindings() << relativePath).firstItem().toInt();
}


QString QueryHelper::categoryForId(int id)
{
    return executeQuery("SELECT name FROM category WHERE id=%s",
                        Bindings() << id).firstItem().toString();
}

int QueryHelper::idForCategory(const QString& category)
{
    return executeQuery("SELECT id FROM category WHERE name=%s",
                        Bindings() << category).firstItem().toInt();
}

QStringList QueryHelper::membersOfCategory(const QString& category)
{
    return executeQuery("SELECT tag.name FROM tag,category "
                        "WHERE tag.categoryId=category.id AND "
                        "category.name=%s",
                        Bindings() << category).asStringList();
}

QValueList<int> QueryHelper::allMediaItemIds()
{
    return executeQuery("SELECT id FROM media").asIntegerList();
}

bool QueryHelper::getMediaItem(int id, DB::ImageInfo& info)
{
    KexiDB::Cursor* c =
        executeQuery("SELECT filename, md5sum, type, label, description, "
                     "startTime, endTime, width, height, angle "
                     "FROM media WHERE id=%s", Bindings() << id).cursor();
    if (!c)
        return false;

    c->moveFirst();
    if (c->eof()) {
        _connection->deleteCursor(c);
        return false;
    }

    info.setFileName(c->value(0).toString());
    info.setMD5Sum(c->value(1).toString());
    info.setMediaType(static_cast<DB::MediaType>(c->value(2).toInt()));
    info.setLabel(c->value(3).toString());
    info.setDescription(c->value(4).toString());
    QDateTime startDate = c->value(5).toDateTime();
    QDateTime endDate = c->value(6).toDateTime();
    info.setDate(DB::ImageDate(startDate, endDate));
    int width = c->value(7).toInt(); // TODO: handle NULL
    int height = c->value(8).toInt(); // TODO: handle NULL
    info.setSize(QSize(width, height));
    info.setAngle(c->value(9).toInt());

    _connection->deleteCursor(c);

    c = executeQuery("SELECT category.name, tag.name "
                     "FROM media_tag, category, tag "
                     "WHERE media_tag.tagId=tag.id AND "
                     "category.id=tag.categoryId AND "
                     "media_tag.mediaId=%s", Bindings() << id).cursor();
    if (!c)
        return false;

    for (c->moveFirst(); !c->eof(); c->moveNext())
        info.addOption(c->value(0).toString(), c->value(1).toString());

    _connection->deleteCursor(c);

    return true;
}

int QueryHelper::insertTag(int categoryId, QString name)
{
    QVariant i = executeQuery("SELECT id FROM tag "
                              "WHERE categoryId=%s AND name=%s",
                              Bindings() << categoryId << name).firstItem();
    if (!i.isNull())
        return i.toInt();

    return insert("tag", "id", QStringList() << "categoryId" << "name",
                  Bindings() << categoryId << name);
}

void QueryHelper::insertMediaTag(int mediaId, int tagId)
{
    if (executeQuery("SELECT mediaId FROM media_tag "
                     "WHERE mediaId=%s AND tagId=%s",
                     Bindings() << mediaId << tagId).
        asIntegerList().count() > 0)
        return;
    executeStatement("INSERT INTO media_tag(mediaId, tagId) "
                     "VALUES(%s, %s)", Bindings() << mediaId << tagId);
}

void QueryHelper::insertMediaItemTags(int mediaId, const DB::ImageInfo& info)
{
    QStringList categories = info.availableCategories();
    for(QStringList::const_iterator categoryIt = categories.begin();
        categoryIt != categories.end(); ++categoryIt) {
        if (*categoryIt == "Folder")
            continue;
        int cid = idForCategory(*categoryIt);
        QStringList items = info.itemsOfCategory(*categoryIt);
        for(QStringList::const_iterator itemIt = items.begin();
            itemIt != items.end(); ++itemIt) {
            Q_ULLONG tagId = insertTag(cid, *itemIt);
            insertMediaTag(mediaId, tagId);
        }
    }
}

void QueryHelper::insertMediaItem(const DB::ImageInfo& info)
{
    // TODO: remove debug
    qDebug("Inserting info of file %s", info.fileName().local8Bit().data());

    QVariant md5 = info.MD5Sum();
    if (md5.toString() == "")
        md5 = QVariant();
    QVariant w = info.size().width();
    if (w.toInt() == -1)
        w = QVariant();
    QVariant h =  info.size().height();
    if (h.toInt() == -1)
        h = QVariant();

    Q_ULLONG mediaId = insert("media", "id", QStringList() <<
                              "filename" << "md5sum" <<
                              "type" << "label" <<
                              "description" <<
                              "startTime" << "endTime" <<
                              "width" << "height" << "angle",
                              Bindings() << info.fileName(true) << md5 <<
                              info.mediaType() << info.label() <<
                              info.description() <<
                              info.date().start() << info.date().end() <<
                              w << h << info.angle());

    insertMediaItemTags(mediaId, info);
}

void QueryHelper::updateMediaItem(int id, const DB::ImageInfo& info)
{
    qDebug("Updating info of file %s", info.fileName().local8Bit().data());

    QVariant md5 = info.MD5Sum();
    if (md5.toString() == "")
        md5 = QVariant();
    QVariant w = info.size().width();
    if (w.toInt() == -1)
        w = QVariant();
    QVariant h =  info.size().height();
    if (h.toInt() == -1)
        h = QVariant();

    executeStatement("UPDATE media SET filename=%s, md5sum=%s, "
                     "type=%s, label=%s, description=%s, "
                     "startTime=%s, endTime=%s, "
                     "width=%s, height=%s, angle=%s WHERE id=%s",
                     Bindings() << info.fileName(true) << md5 <<
                     info.mediaType() << info.label() << info.description() <<
                     info.date().start() << info.date().end() <<
                     w << h << info.angle() << id);

    insertMediaItemTags(id, info);
}

#endif /* HASKEXIDB */
