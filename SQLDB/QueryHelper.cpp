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

#include "QueryHelper.h"
#include "KexiHelpers.h"
#include "QueryErrors.h"
#include "Settings/SettingsData.h"
#include "Viewer/CircleDraw.h"
#include "Viewer/LineDraw.h"
#include "Viewer/RectDraw.h"
#include <kexidb/transaction.h>
#include <klocale.h>
#include <qsize.h>

using namespace SQLDB;

QueryHelper::Result::Result(KexiDB::Cursor* cursor):
    _cursor(cursor)
{
}

QueryHelper::Result::~Result()
{
    if (_cursor) {
        KexiDB::Connection* connection = _cursor->connection();
        if (connection)
            connection->deleteCursor(_cursor);
        else {
            _cursor->close();
            delete _cursor;
        }
    }
}

QStringList QueryHelper::Result::asStringList()
{
    QStringList r;
    if (_cursor) {
        r = readStringsFromCursor(*_cursor);
    }
    return r;
}

QValueList<QString[2]> QueryHelper::Result::asString2List()
{
    QValueList<QString[2]> r;
    if (_cursor) {
        r = readString2sFromCursor(*_cursor);
    }
    return r;
}

QValueList<QString[3]> QueryHelper::Result::asString3List()
{
    QValueList<QString[3]> r;
    if (_cursor) {
        r = readString3sFromCursor(*_cursor);
    }
    return r;
}

QValueList<int> QueryHelper::Result::asIntegerList()
{
    QValueList<int> r;
    if (_cursor) {
        r = readIntsFromCursor(*_cursor);
    }
    return r;
}

QValueList< QPair<int, QString> > QueryHelper::Result::asIntegerStringPairs()
{
    QValueList< QPair<int, QString> > r;
    if (_cursor) {
        for (_cursor->moveFirst(); !_cursor->eof(); _cursor->moveNext())
            r << QPair<int, QString>(_cursor->value(0).toInt(),
                                     _cursor->value(1).toString());
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
    }
    return r;
}

RowData QueryHelper::Result::getRow(uint n)
{
    if (_cursor) {
        _cursor->moveFirst();
        for (uint i = 0; i < n; ++i) {
            if (!_cursor->moveNext())
                break;
        }
        if (!_cursor->eof()) {
            RowData r(_cursor->fieldCount());
            _cursor->storeCurrentRow(r);
            return r;
        }
    }
    throw Error(/* TODO: type and message */);
}

Cursor QueryHelper::Result::cursor()
{
    KexiDB::Cursor* c = _cursor;
    _cursor = 0;
    return Cursor(c);
}


QueryHelper* QueryHelper::_instance = 0;

QueryHelper::QueryHelper(KexiDB::Connection& connection):
    _connection(&connection),
    _driver(connection.driver())
{
    if (!_driver)
        throw Error(/* TODO: error type and message */);
}

void QueryHelper::setup(KexiDB::Connection& connection)
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

QString QueryHelper::getSQLRepresentation(const QVariant& x)
{
    if (x.type() == QVariant::List) {
        QStringList repr;
        QValueList<QVariant> l = x.toList();
        for (QValueList<QVariant>::const_iterator i = l.begin();
             i != l.end(); ++i) {
            repr << getSQLRepresentation(*i);
        }
        if (repr.count() == 0)
            return "NULL";
        else
            return repr.join(", ");
    }
    else
        // Escape and convert x to string
        return _driver->valueToSQL(fieldTypeFor(x), x);
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
        s = s.replace(n, 2, getSQLRepresentation(*i));
    }
}

void QueryHelper::executeStatement(const QString& statement,
                                   const Bindings& bindings)
{
    QString s = statement;
    bindValues(s, bindings);

    //TODO: remove debug
    qDebug("Executing statement: %s", s.local8Bit().data());

    if (!_connection->executeSQL(s))
        throw SQLError(_connection->recentSQLString(),
                       _connection->errorMsg());
}

QueryHelper::Result QueryHelper::executeQuery(const QString& query,
                                              const Bindings& bindings)
{
    QString q = query;
    bindValues(q, bindings);

    //TODO: remove debug
    qDebug("Executing query: %s", q.local8Bit().data());

    KexiDB::Cursor* c = _connection->executeQuery(q);
    if (!c) {
        throw SQLError(_connection->recentSQLString(),
                       _connection->errorMsg());
    }
    return Result(c);
}

Q_ULLONG QueryHelper::insert(const QString& tableName,
                             const QString& aiFieldName,
                             const QStringList& fields,
                             const Bindings& values)
{
    Q_ASSERT(fields.count() == values.count());

    QString q = "INSERT INTO %1(%2) VALUES (%3)";
    q = q.arg(tableName);
    q = q.arg(fields.join(", "));
    QStringList l;
    for (Bindings::size_type i = 0; i < values.count(); ++i)
        l.append("%s");
    q = q.arg(l.join(", "));
    executeStatement(q, values);
    return _connection->lastInsertedAutoIncValue(aiFieldName, tableName);
}

namespace
{
void splitPath(const QString& fullname, QString& path, QString& filename)
{
    int i = fullname.findRev("/");
    if (i == -1) {
        path = ".";
        filename = fullname;
    }
    else {
        // FIXME: anything special needed if fullname is form "/foo.jpg"?
        path = fullname.left(i);
        filename = fullname.mid(i+1);
    }
}

QString makeFullName(const QString& path, const QString& filename)
{
    if (path == ".")
        return filename;
    else
        return path + "/" + filename;
}
}

QStringList QueryHelper::relativeFilenames()
{
    QValueList<QString[2]> dirFilePairs = QueryHelper::instance()->
        executeQuery("SELECT dir.path, media.filename FROM dir, media "
                     "WHERE media.dirId=dir.id ORDER BY media.place"
                     ).asString2List();
    QStringList r;
    for (QValueList<QString[2]>::const_iterator i = dirFilePairs.begin();
         i != dirFilePairs.end(); ++i) {
        r << makeFullName((*i)[0], (*i)[1]);
    }
    return r;
}

QString QueryHelper::filenameForId(int id, bool fullPath)
{
    QValueList<QString[2]> dirFilePairs =
        executeQuery("SELECT dir.path, media.filename FROM dir, media "
                     "WHERE dir.id=media.dirId AND media.id=%s",
                     Bindings() << id).asString2List();
    if (dirFilePairs.count() == 0)
        throw NotFoundError(/* TODO: message */);

    QString fn = makeFullName(dirFilePairs[0][0], dirFilePairs[0][1]);

    if (fullPath)
        return Settings::SettingsData::instance()->imageDirectory() + fn;
    else
        return fn;
}

int QueryHelper::idForFilename(const QString& relativePath)
{
    QString path;
    QString filename;
    splitPath(relativePath, path, filename);
    QVariant id =
        executeQuery("SELECT media.id FROM media, dir "
                     "WHERE media.dirId=dir.id AND "
                     "dir.path=%s AND media.filename=%s",
                     Bindings() << path << filename).firstItem();
    if (id.isNull())
        throw NotFoundError(i18n("Media item for file %1 cannot be found "
                                 "from the SQL database").arg(filename));
    return id.toInt();
}

QValueList<int> QueryHelper::idsForFilenames(const QStringList& relativePaths)
{
#if 1
    QStringList paths;
    QStringList filenames;
    QMap<QString, int> pathIdMap;
    for (QStringList::const_iterator i = relativePaths.begin();
         i != relativePaths.end(); ++i) {
        QString path;
        QString filename;
        splitPath(*i, path, filename);
        paths << path;
        filenames << filename;
        if (!pathIdMap.contains(path)) {
            pathIdMap.insert(path, executeQuery("SELECT id FROM dir "
                                                "WHERE path=%s",
                                                Bindings() << path
                                                ).firstItem().toInt());
        }
    }
    QValueList<int> idList;
    QStringList::const_iterator pathIt = paths.begin();
    QStringList::const_iterator filenameIt = filenames.begin();
    while (pathIt != paths.end()) {
        QVariant id =
            executeQuery("SELECT id FROM media WHERE dirId=%s AND filename=%s",
                         Bindings() << pathIdMap[*pathIt] << *filenameIt
                         ).firstItem();
        /*
        if (id.isNull())
            throw NotFoundError(i18n("Media item for file %1 cannot be found "
                                     "from the SQL database").arg(*filenameIt));
        */
        idList << id.toInt();
        ++pathIt;
        ++filenameIt;
    }
    return idList;
#else
    // Uses CONCAT function, which is MySQL specific.

    QStringList adjustedPaths = relativePaths;
    for (QStringList::iterator i = adjustedPaths.begin(); i != adjustedPaths.end(); ++i) {
        if ((*i).find('/') == -1) {
            *i = QString::fromLatin1("./") + *i;
        }
    }
    executeQuery("SELECT id FROM media, dir WHERE CONCAT(dir.path, '/', media.filename) IN (%s)");
#endif
}

QString QueryHelper::categoryForId(int id)
{
    QVariant r = executeQuery("SELECT name FROM category WHERE id=%s",
                              Bindings() << id).firstItem();

    if (r.isNull())
        throw NotFoundError(/* TODO: message */);
    else
        return r.toString();
}

int QueryHelper::idForCategory(const QString& category)
{
    QVariant r = executeQuery("SELECT id FROM category WHERE name=%s",
                              Bindings() << category).firstItem();
    if (r.isNull())
        throw NotFoundError(/* TODO: message */);
    else
        return r.toInt();
}

QValueList<int> QueryHelper::tagIdsOfCategory(const QString& category)
{
    return executeQuery("SELECT tag.id FROM tag,category "
                        "WHERE tag.categoryId=category.id AND "
                        "category.name=%s",
                        Bindings() << category).asIntegerList();
}

QStringList QueryHelper::membersOfCategory(int categoryId)
{
    // Tags with larger place come first. (NULLs last)
    return executeQuery("SELECT name FROM tag "
                        "WHERE categoryId=%s ORDER BY place DESC",
                        Bindings() << categoryId).asStringList();
}

QStringList QueryHelper::membersOfCategory(const QString& category)
{
    return executeQuery("SELECT tag.name FROM tag,category "
                        "WHERE tag.categoryId=category.id AND "
                        "category.name=%s",
                        Bindings() << category).asStringList();
}

QStringList QueryHelper::folders()
{
    return executeQuery("SELECT path FROM dir").asStringList();
}

QValueList<int> QueryHelper::allMediaItemIds()
{
    return executeQuery("SELECT id FROM media ORDER BY place").asIntegerList();
}

QValueList<int> QueryHelper::allMediaItemIdsByType(int typemask)
{
    return executeQuery("SELECT id FROM media WHERE type&%s!=0 ORDER BY place",
                        Bindings() << typemask).asIntegerList();
}

void QueryHelper::getMediaItem(int id, DB::ImageInfo& info)
{
    RowData row =
        executeQuery("SELECT dir.path, m.filename, m.md5sum, m.type, "
                     "m.label, m.description, "
                     "m.startTime, m.endTime, m.width, m.height, m.angle "
                     "FROM media m, dir "
                     "WHERE m.dirId=dir.id AND "
                     "m.id=%s", Bindings() << id).getRow();

    if (row.count() != 11)
        throw Error(/* TODO: error type and message */);

    info.delaySavingChanges(true);

    info.setFileName(makeFullName(row[0].toString(), row[1].toString()));
    info.setMD5Sum(row[2].toString());
    info.setMediaType(static_cast<DB::MediaType>(row[3].toInt()));
    info.setLabel(row[4].toString());
    info.setDescription(row[5].toString());
    QDateTime startDate = row[6].toDateTime();
    QDateTime endDate = row[7].toDateTime();
    info.setDate(DB::ImageDate(startDate, endDate));
    int width = row[8].isNull() ? -1 : row[8].toInt();
    int height = row[9].isNull() ? -1 :row[9].toInt();
    info.setSize(QSize(width, height));
    info.setAngle(row[10].toInt());

    QValueList<QString[2]> categoryTagPairs =
        executeQuery("SELECT category.name, tag.name "
                     "FROM media_tag, category, tag "
                     "WHERE media_tag.tagId=tag.id AND "
                     "category.id=tag.categoryId AND "
                     "media_tag.mediaId=%s", Bindings() << id).asString2List();

    for (QValueList<QString[2]>::const_iterator i = categoryTagPairs.begin();
         i != categoryTagPairs.end(); ++i)
        info.addOption((*i)[0], (*i)[1]);

    Viewer::DrawList drawList;
    Cursor c = executeQuery("SELECT shape, x0, y0, x1, y1 "
                            "FROM drawing WHERE mediaId=%s",
                            Bindings() << id).cursor();
    for (c.selectFirstRow(); c.rowExists(); c.selectNextRow()) {
        RowData row;
        c.getCurrentRow(row);
        Viewer::Draw* drawing;
        switch (row[0].toInt()) {
        case 0:
            drawing = new Viewer::CircleDraw();
            break;
        case 1:
            drawing = new Viewer::LineDraw();
            break;
        case 2:
            drawing = new Viewer::RectDraw();
            break;
        default:
            continue;
        }
        drawing->setCoordinates(QPoint(row[1].toInt(),
                                       row[2].toInt()),
                                QPoint(row[3].toInt(),
                                       row[4].toInt()));
        drawList << drawing;
    }
    info.setDrawList(drawList);

    // TODO: remove debug
    qDebug("Read info of file %s (id %d)", info.fileName().local8Bit().data(), id);
}

int QueryHelper::insertTag(int categoryId, const QString& name)
{
    QVariant i = executeQuery("SELECT id FROM tag "
                              "WHERE categoryId=%s AND name=%s",
                              Bindings() << categoryId << name).firstItem();
    if (!i.isNull())
        return i.toInt();

    return insert("tag", "id", QStringList() << "categoryId" << "name",
                  Bindings() << categoryId << name);
}

void QueryHelper::insertTagFirst(int categoryId, const QString& name)
{
    // See membersOfCategory() for tag ordering usage.

    int id = insertTag(categoryId, name);
    QVariant oldPlace = executeQuery("SELECT place FROM tag WHERE id=%s",
                                     Bindings() << id).firstItem();

    // Move tags from this to previous first tag one place towards end
    if (!oldPlace.isNull()) {
        executeStatement("UPDATE tag SET place=place-1 "
                         "WHERE categoryId=%s AND place>%s",
                         Bindings() << categoryId << oldPlace);
    }

    // MAX(place) could be NULL, but it'll be returned as 0 with
    // toInt(), which is ok.
    int newPlace = executeQuery("SELECT MAX(place) FROM tag "
                                "WHERE categoryId=%s",
                                Bindings() << categoryId
                                ).firstItem().toInt() + 1;

    // Put this tag first
    executeStatement("UPDATE tag SET place=%s WHERE id=%s",
                     Bindings() << newPlace << id);
}

void QueryHelper::removeTag(int categoryId, const QString& name)
{
#ifndef USEFOREIGNCONSTRAINT
    int tagId =
        executeQuery("SELECT id FROM tag WHERE categoryId=%s AND name=%s",
                     Bindings() << categoryId << name).firstItem().toInt();
    executeStatement("DELETE FROM media_tag WHERE tagId=%s",
                     Bindings() << tagId);
#endif
    executeStatement("DELETE FROM tag WHERE categoryId=%s AND name=%s",
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

void QueryHelper::insertMediaItemDrawings(int mediaId,
                                          const DB::ImageInfo& info)
{
    Viewer::DrawList drawings = info.drawList();
    for (Viewer::DrawList::const_iterator i = drawings.begin();
        i != drawings.end(); ++i) {
        int shape;
        if (dynamic_cast<Viewer::CircleDraw*>(*i)) {
            shape = 0;
        }
        else if (dynamic_cast<Viewer::LineDraw*>(*i)) {
            shape = 1;
        }
        else if (dynamic_cast<Viewer::RectDraw*>(*i)) {
            shape = 2;
        }
        else
            continue;

        QPoint p0 = (*i)->getStartPoint();
        QPoint p1 = (*i)->getEndPoint();

        executeStatement("INSERT INTO drawing "
                         "(mediaId, shape, x0, y0, x1, y1) "
                         "VALUES (%s, %s, %s, %s, %s, %s)",
                         Bindings() << mediaId << shape <<
                         p0.x() << p0.y() << p1.x() << p1.y());
    }
}

int QueryHelper::insertDir(const QString& relativePath)
{
    QVariant i = executeQuery("SELECT id FROM dir "
                              "WHERE path=%s",
                              Bindings() << relativePath).firstItem();
    if (!i.isNull())
        return i.toInt();

    return insert("dir", "id", QStringList() << "path",
                  Bindings() << relativePath);
}

/** Get data from ImageInfo to Bindings list.
 *
 * Adds also image directory to database, if it's not already there.
 *
 * @return Bingings in following order:
 *  dirId, filename, md5sum, type, label, description,
 *  startTime, endTime, width, height, angle
 */
QueryHelper::Bindings
QueryHelper::imageInfoToBindings(const DB::ImageInfo& info)
{
    //Q_ASSERT(bindings.isEmpty());

    QVariant md5 = info.MD5Sum();
    if (md5.toString().isEmpty())
        md5 = QVariant();
    QVariant w = info.size().width();
    if (w.toInt() == -1)
        w = QVariant();
    QVariant h =  info.size().height();
    if (h.toInt() == -1)
        h = QVariant();
    QString path;
    QString filename;
    splitPath(info.fileName(true), path, filename);

    int dirId = insertDir(path);

    return Bindings() <<
        dirId << filename << md5 <<
        info.mediaType() << info.label() <<
        info.description() <<
        info.date().start() << info.date().end() <<
        w << h << info.angle();
}

void QueryHelper::insertMediaItem(const DB::ImageInfo& info, int place)
{
    // TODO: remove debug
    qDebug("Inserting info of file %s", info.fileName().local8Bit().data());

    QStringList fields;
    Bindings bindings;
    if (place != 0) {
        fields << "place";
        bindings << place;
    }
    fields <<
        "dirId" << "filename" << "md5sum" <<
        "type" << "label" <<
        "description" <<
        "startTime" << "endTime" <<
        "width" << "height" << "angle";
    bindings << imageInfoToBindings(info);

    Q_ULLONG mediaId = insert("media", "id", fields, bindings);

    insertMediaItemTags(mediaId, info);
    insertMediaItemDrawings(mediaId, info);
}

void
QueryHelper::insertMediaItemsLast(const QValueList<DB::ImageInfoPtr>& items)
{
    int place =
        executeQuery("SELECT MAX(place) FROM media").firstItem().toInt() + 1;

    for (QValueList<DB::ImageInfoPtr>::const_iterator i = items.constBegin();
         i != items.constEnd(); ++i) {
        insertMediaItem(*(*i), place++);
    }
}

void QueryHelper::updateMediaItem(int id, const DB::ImageInfo& info)
{
    // TODO: remove debug
    qDebug("Updating info of file %s", info.fileName().local8Bit().data());

    executeStatement("UPDATE media SET dirId=%s, filename=%s, md5sum=%s, "
                     "type=%s, label=%s, description=%s, "
                     "startTime=%s, endTime=%s, "
                     "width=%s, height=%s, angle=%s WHERE id=%s",
                     imageInfoToBindings(info) << id);

    executeStatement("DELETE FROM media_tag WHERE mediaId=%s",
                     Bindings() << id);
    insertMediaItemTags(id, info);

    executeStatement("DELETE FROM drawing WHERE mediaId=%s",
                     Bindings() << id);
    insertMediaItemDrawings(id, info);
}

QValueList<int> QueryHelper::getDirectMembers(int tagId)
{
    return executeQuery("SELECT fromTagId FROM tag_relation WHERE toTagId=%s",
                        Bindings() << tagId).asIntegerList();
}

int QueryHelper::idForTag(const QString& category, const QString& item)
{
    return executeQuery("SELECT tag.id FROM tag,category "
                        "WHERE tag.categoryId=category.id AND "
                        "category.name=%s AND tag.name=%s",
                        Bindings() << category << item).firstItem().toInt();
}

QValueList<int> QueryHelper::idListForTag(const QString& category,
                                          const QString& item)
{
    int tagId = idForTag(category, item);
    QValueList<int> visited, queue;
    visited << tagId;
    queue << tagId;
    while (!queue.empty()) {
        QValueList<int> adj = getDirectMembers(queue.first());
        queue.pop_front();
        QValueListConstIterator<int> adjEnd(adj.constEnd());
        for (QValueList<int>::const_iterator a = adj.constBegin();
             a != adjEnd; ++a) {
            if (!visited.contains(*a)) {
                queue << *a;
                visited << *a;
            }
        }
    }
    return visited;
}

void QueryHelper::addBlockItem(const QString& relativePath)
{
    QString path;
    QString fn;
    splitPath(relativePath, path, fn);
    int dirId = insertDir(path);
    if (executeQuery("SELECT COUNT(*) FROM blockitem "
                     "WHERE dirId=%s AND filename=%s",
                     Bindings() << dirId << fn).firstItem().asInt() == 0)
        executeStatement("INSERT INTO blockitem(dirId, filename) "
                         "VALUES(%s, %s)", Bindings() << dirId << fn);
}

void QueryHelper::addBlockItems(const QStringList& relativePaths)
{
    for (QStringList::const_iterator i = relativePaths.begin();
         i != relativePaths.end(); ++i)
        addBlockItem(*i);
}

bool QueryHelper::isBlocked(const QString& relativePath)
{
    QString path;
    QString fn;
    splitPath(relativePath, path, fn);
    return executeQuery("SELECT COUNT(*) FROM blockitem, dir "
                        "WHERE blockitem.dirId=dir.id AND "
                        "dir.path=%s AND blockitem.filename=%s",
                        Bindings() << path << fn).firstItem().toInt() > 0;
}

void QueryHelper::removeMediaItem(const QString& relativePath)
{
    QString path;
    QString fn;
    splitPath(relativePath, path, fn);
    int id = executeQuery("SELECT id FROM media "
                          "WHERE dirId=(SELECT id FROM dir WHERE path=%s) AND "
                          "filename=%s",
                          Bindings() << path << fn).firstItem().asInt();
    executeStatement("DELETE FROM media_tag WHERE mediaId=%s",
                     Bindings() << id);
    executeStatement("DELETE FROM media WHERE id=%s", Bindings() << id);
}

bool QueryHelper::containsMD5Sum(const QString& md5sum)
{
    return executeQuery("SELECT count(*) FROM media WHERE md5sum=%s",
                        Bindings() << md5sum).firstItem().toInt();
}

QString QueryHelper::filenameForMD5Sum(const QString& md5sum)
{
    QValueList<QString[2]> rows =
        executeQuery("SELECT dir.path, media.filename FROM dir, media "
                     "WHERE dir.id=media.dirId AND media.md5sum=%s",
                     Bindings() << md5sum).asString2List();
    if (rows.count() == 0)
        return QString::null;
    else {
        return makeFullName(rows[0][0], rows[0][1]);
    }
}

QValueList< QPair<int, QString> >
QueryHelper::getMediaIdTagPairs(const QString& category, int typemask)
{
    if (category == "Folder")
        return executeQuery("SELECT media.id, dir.path FROM media, dir "
                            "WHERE media.dirId=dir.id AND media.type&%s!=0",
                            Bindings() << typemask).asIntegerStringPairs();
    else
        return executeQuery("SELECT media.id, tag.name "
                            "FROM media, media_tag, tag, category "
                            "WHERE media.id=media_tag.mediaId AND "
                            "media_tag.tagId=tag.id AND "
                            "tag.categoryId=category.id AND "
                            "media.type&%s!=0 AND category.name=%s",
                            Bindings() << typemask << category
                            ).asIntegerStringPairs();
}

int QueryHelper::mediaPlaceByFilename(const QString& relativePath)
{
    QString path;
    QString filename;
    splitPath(relativePath, path, filename);
    QVariant place =
        executeQuery("SELECT media.place FROM media, dir "
                     "WHERE media.dirId=dir.id AND "
                     "dir.path=%s AND media.filename=%s",
                     Bindings() << path << filename).firstItem();
    if (place.isNull())
        throw NotFoundError(/* TODO: message */);
    return place.toInt();
}

/** Move sourceItems after or before destination item.
 */
void QueryHelper::moveMediaItems(const QStringList& sourceItems,
                                 const QString& destination, bool after)
{

    // BROKEN!
    // TODO: make this function work!


    if (sourceItems.isEmpty())
        return;

    QValueList<int> srcIds;
    for (QStringList::const_iterator i = sourceItems.constBegin();
         i != sourceItems.constEnd(); ++i) {
        srcIds << idForFilename(*i);
    }

    RowData minmax =
        executeQuery("SELECT MIN(place), MAX(place) "
                     "FROM media WHERE id IN (%s)",
                     Bindings() << toVariantList(srcIds)).getRow();
    int srcMin = minmax[0].toInt();
    int srcMax = minmax[1].toInt();

    // Move media items which are between srcIds just before first
    // item in srcIds list.
    QValueList<int> betweenList =
        executeQuery("SELECT id FROM media "
                     "WHERE %s <= place AND place <= %s AND id NOT IN (%s)",
                     Bindings() << srcMin << srcMax << toVariantList(srcIds)
                     ).asIntegerList();
    int n = srcMin;
    for (QValueList<int>::const_iterator i = betweenList.begin();
         i != betweenList.end(); ++i) {
        executeStatement("UPDATE media SET place=%s WHERE id=%s",
                         Bindings() << n << *i);
        ++n;
    }

    // Make items in srcIds continuous, so places of items are
    // N, N+1, N+2, ..., N+n.
    for (QValueList<int>::const_iterator i = srcIds.begin();
         i != srcIds.end(); ++i) {
        executeStatement("UPDATE media SET place=%s WHERE id=%s",
                         Bindings() << n << *i);
        ++n;
    }

    int destPlace =
        mediaPlaceByFilename(destination) + (after ? 1 : 0);

    if (srcMin <= destPlace && destPlace <= srcMax)
        return;
    int low = destPlace <= srcMin ? destPlace : srcMin;
    int high = destPlace >= srcMax ? destPlace : srcMax;

    Q_ASSERT(low == destPlace || high == destPlace);

    uint N = srcIds.count();

    if (low < destPlace)
        executeStatement("UPDATE media SET place=place-%s "
                         "WHERE %s <= place AND place <= %s",
                         Bindings() << N << low << destPlace - 1);
    else if (destPlace < high)
        executeStatement("UPDATE media SET place=place+%s "
                         "WHERE %s <= place AND place <= %s",
                         Bindings() << N << destPlace << high - 1);

    executeStatement("UPDATE media SET place=place+(%s) WHERE id IN (%s)",
                     Bindings() << destPlace - srcMin << toVariantList(srcIds));
}

void QueryHelper::makeMediaPlacesContinuous()
{
    executeStatement("UPDATE media, "
                     "(SELECT a.id AS id, COUNT(*) AS n "
                     "FROM media a, media b "
                     "WHERE "
                     "(CASE b.place "
                     "WHEN a.place THEN b.id "
                     "ELSE b.place END) "
                     "<= "
                     "(CASE b.place "
                     "WHEN a.place THEN a.id "
                     "ELSE a.place END) "
                     "GROUP BY a.id) x "
                     "SET media.place=x.n WHERE media.id=x.id");
}

void QueryHelper::sortMediaItems(const QStringList& relativePaths)
{
    KexiDB::TransactionGuard transaction(*_connection);

    QValueList<int> idList = idsForFilenames(relativePaths);

#if 0
    QValueList<QVariant> x = toVariantList(idList);

    executeStatement("UPDATE media, "
                     "(SELECT a.place AS place, COUNT(*) AS n "
                     " FROM media a, media b "
                     " WHERE a.id IN (%s) AND b.id IN (%s) AND "
                     " b.place <= a.place GROUP BY a.id) byplace, "
                     "(SELECT a.id AS id, COUNT(*) AS n "
                     " FROM media a, media b "
                     " WHERE a.id IN (%s) AND b.id IN (%s) AND "
                     " b.startTime <= a.startTime GROUP BY a.id) bytime "
                     "SET media.place=byplace.place "
                     "WHERE media.id=bytime.id AND byplace.n=bytime.n",
                     Bindings() << x << x << x << x);
#else
    executeStatement("CREATE TEMPORARY TABLE sorttmp "
                     "SELECT id, place, startTime "
                     "FROM media WHERE id IN (%s)",
                     Bindings() << toVariantList(idList));

    idList =
        executeQuery("SELECT id FROM sorttmp "
                     "ORDER BY startTime").asIntegerList();
    QValueList<int> placeList =
        executeQuery("SELECT place FROM sorttmp "
                     "ORDER BY place").asIntegerList();

    QValueList<int>::const_iterator idIt = idList.begin();
    QValueList<int>::const_iterator placeIt = placeList.begin();
    for (; idIt != idList.end(); ++idIt, ++placeIt) {
        executeStatement("UPDATE sorttmp SET place=%s WHERE id=%s",
                         Bindings() << *placeIt << *idIt);

    }

    executeStatement("UPDATE media, sorttmp t "
                     "SET media.place=t.place WHERE media.id=t.id");

    executeStatement("DROP TABLE sorttmp");
#endif

    transaction.commit();
}

QString QueryHelper::findFirstFileInTimeRange(const DB::ImageDate& range,
                                              bool includeRanges)
{
    return findFirstFileInTimeRange(range, includeRanges, 0);
}

QString QueryHelper::findFirstFileInTimeRange(const DB::ImageDate& range,
                                              bool includeRanges,
                                              const QValueList<int>& idList)
{
    return findFirstFileInTimeRange(range, includeRanges, &idList);
}

QString QueryHelper::findFirstFileInTimeRange(const DB::ImageDate& range,
                                              bool includeRanges,
                                              const QValueList<int>* idList)
{
    QString query =
        "SELECT dir.path, media.filename FROM media, dir "
        "WHERE dir.id=media.dirId AND ";
    Bindings bindings;

    if (idList) {
        query += "media.id IN (%s) AND ";
        bindings << toVariantList(*idList);
    }

    if (!includeRanges) {
        query += "%s <= media.startTime AND media.endTime <= %s";
    }
    else {
        query += "%s <= media.endTime AND media.startTime <= %s";
    }
    bindings << range.start() << range.end();

    query += " ORDER BY media.startTime LIMIT 1";

    QValueList<QString[2]> dirFilenamePairs =
        executeQuery(query, bindings).asString2List();

    if (dirFilenamePairs.isEmpty())
        return QString::null;
    else {
        return makeFullName(dirFilenamePairs[0][0], dirFilenamePairs[0][1]);
    }
}
