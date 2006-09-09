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
#include "QueryErrors.h"
#include "Viewer/CircleDraw.h"
#include "Viewer/LineDraw.h"
#include "Viewer/RectDraw.h"
#include "DB/ImageSearchInfo.h"
#include "DB/CategoryMatcher.h"
#include "Utilities/List.h"
#include <kexidb/transaction.h>
#include <klocale.h>
#include <qsize.h>

using namespace SQLDB;
using Utilities::mergeListsUniqly;
using Utilities::listSubtract;

QueryHelper::QueryHelper(Connection& connection):
    _connection(&connection),
    _driver(connection.driver())
{
    if (!_driver)
        throw InitializationError();
}

namespace
{
    KexiDB::Field::Type fieldTypeFor(QVariant::Type type)
    {
        switch (type) {
        case QVariant::Bool:
            return KexiDB::Field::Boolean;

        case QVariant::Int:
        case QVariant::UInt:
            return KexiDB::Field::Integer;

        case QVariant::LongLong:
        case QVariant::ULongLong:
            return KexiDB::Field::BigInteger;

        case QVariant::Double:
            return KexiDB::Field::Double;

        case QVariant::Date:
            return KexiDB::Field::Date;

        case QVariant::Time:
            return KexiDB::Field::Time;

        case QVariant::DateTime:
            return KexiDB::Field::DateTime;

        case QVariant::String:
        case QVariant::CString:
            return KexiDB::Field::Text;

        default:
            return KexiDB::Field::InvalidType;
        }
    }
}

QString QueryHelper::sqlRepresentation(const QVariant& x) const
{
    if (x.type() == QVariant::List) {
        QStringList repr;
        QValueList<QVariant> l = x.toList();
        for (QValueList<QVariant>::const_iterator i = l.begin();
             i != l.end(); ++i) {
            repr << sqlRepresentation(*i);
        }
        if (repr.isEmpty())
            return "NULL";
        else
            return repr.join(", ");
    }
    else
        // Escape and convert x to string
        return _driver->valueToSQL(fieldTypeFor(x.type()), x);
}

void QueryHelper::bindValues(QString &s, const Bindings& b) const
{
    int n = 0;
    for (Bindings::const_iterator i = b.begin(); i != b.end(); ++i) {
        do {
            n = s.find("%s", n);
        } while (n >= 1 && s.at(n - 1) == '%');
        if (n == -1)
            break;
        s = s.replace(n, 2, sqlRepresentation(*i));
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
        throw StatementError(_connection->recentSQLString(),
                             _connection->errorMsg());
}

QueryResult QueryHelper::executeQuery(const QString& query,
                                      const Bindings& bindings) const
{
    QString q = query;
    bindValues(q, bindings);

    //TODO: remove debug
    qDebug("Executing query: %s", q.local8Bit().data());

    KexiDB::Cursor* c = _connection->executeQuery(q);
    if (!c) {
        throw QueryError(_connection->recentSQLString(),
                         _connection->errorMsg());
    }
    return QueryResult(c);
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
    for (Bindings::size_type i = values.count(); i > 0; --i)
        l.append("%s");
    q = q.arg(l.join(", "));
    executeStatement(q, values);
    return _connection->lastInsertedAutoIncValue(aiFieldName, tableName);
}

namespace
{
void splitPath(const QString& filename, QString& path, QString& basename)
{
    int i = filename.findRev("/");
    if (i == -1) {
        path = ".";
        basename = filename;
    }
    else {
        // FIXME: anything special needed if filename is form "/foo.jpg"?
        path = filename.left(i);
        basename = filename.mid(i + 1);
    }
}

QString makeFullName(const QString& path, const QString& basename)
{
    if (path == ".")
        return basename;
    else
        return path + "/" + basename;
}

QString typeCondition(const QString& fieldName, DB::MediaType typemask)
{
    if (typemask == DB::Image ||
        typemask == DB::Video)
        return fieldName + "=" + QString::number(typemask);
    else if (typemask == DB::anyMediaType)
        return "1=1";
    else {
        QStringList typeList;
        for (uint t = 1; t < DB::anyMediaType; t <<= 1)
            if (typemask & t)
                typeList.append(fieldName + "=" + QString::number(t));
        return "(" + typeList.join(" OR ") + ")";
    }
}
}

QStringList QueryHelper::filenames() const
{
    QValueList<QString[2]> dirFilePairs =
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

QString QueryHelper::mediaItemFilename(int id) const
{
    QValueList<QString[2]> dirFilePairs =
        executeQuery("SELECT dir.path, media.filename FROM dir, media "
                     "WHERE dir.id=media.dirId AND media.id=%s",
                     Bindings() << id).asString2List();
    if (dirFilePairs.isEmpty())
        throw EntryNotFoundError();

    return makeFullName(dirFilePairs[0][0], dirFilePairs[0][1]);
}

int QueryHelper::mediaItemId(const QString& filename) const
{
    QString path;
    QString basename;
    splitPath(filename, path, basename);
    QVariant id =
        executeQuery("SELECT media.id FROM media, dir "
                     "WHERE media.dirId=dir.id AND "
                     "dir.path=%s AND media.filename=%s",
                     Bindings() << path << basename).firstItem();
    if (id.isNull())
        throw EntryNotFoundError();
    return id.toInt();
}

QValueList< QPair<int, QString> > QueryHelper::mediaItemIdFileMap() const
{
    Cursor c = executeQuery("SELECT media.id, dir.path, media.filename "
                            "FROM media, dir "
                            "WHERE media.dirId=dir.id").cursor();
    QValueList< QPair<int, QString> > r;

    if (c.isNull())
        return r;

    for (c.selectFirstRow(); c.rowExists(); c.selectNextRow()) {
        QPair<int, QString> x;
        x.first = c.value(0).toInt();
        x.second = makeFullName(c.value(1).toString(), c.value(2).toString());
        r << x;
    }
    return r;
}

QValueList<int>
QueryHelper::mediaItemIdsForFilenames(const QStringList& filenames) const
{
#if 1
    QStringList paths;
    QStringList basenames;
    QMap<QString, int> pathIdMap;
    for (QStringList::const_iterator i = filenames.begin();
         i != filenames.end(); ++i) {
        QString path;
        QString basename;
        splitPath(*i, path, basename);
        paths << path;
        basenames << basename;
        if (!pathIdMap.contains(path)) {
            pathIdMap.insert(path, executeQuery("SELECT id FROM dir "
                                                "WHERE path=%s",
                                                Bindings() << path
                                                ).firstItem().toInt());
        }
    }
    QValueList<int> idList;
    QStringList::const_iterator pathIt = paths.begin();
    QStringList::const_iterator basenameIt = basenames.begin();
    while (pathIt != paths.end()) {
        QVariant id =
            executeQuery("SELECT id FROM media WHERE dirId=%s AND filename=%s",
                         Bindings() << pathIdMap[*pathIt] << *basenameIt
                         ).firstItem();
        /*
        if (id.isNull())
            throw EntryNotFoundError();
        */
        if (!id.isNull())
            idList << id.toInt();
        ++pathIt;
        ++basenameIt;
    }
    return idList;
#else
    // Uses CONCAT function, which is MySQL specific.

    QStringList adjustedPaths = filenames;
    for (QStringList::iterator i = adjustedPaths.begin(); i != adjustedPaths.end(); ++i) {
        if ((*i).find('/') == -1) {
            *i = QString::fromLatin1("./") + *i;
        }
    }
    executeQuery("SELECT id FROM media, dir WHERE CONCAT(dir.path, '/', media.filename) IN (%s)");
#endif
}

QStringList QueryHelper::categoryNames() const
{
    return executeQuery("SELECT name FROM category").asStringList();
}

int QueryHelper::categoryId(const QString& category) const
{
    QVariant r = executeQuery("SELECT id FROM category WHERE name=%s",
                              Bindings() << category).firstItem();
    if (r.isNull())
        throw EntryNotFoundError();
    else
        return r.toInt();
}

QValueList<int> QueryHelper::tagIdsOfCategory(const QString& category) const
{
    return executeQuery("SELECT tag.id FROM tag, category "
                        "WHERE tag.categoryId=category.id AND "
                        "category.name=%s",
                        Bindings() << category).asIntegerList();
}

QStringList QueryHelper::tagNamesOfCategory(int categoryId) const
{
    // Tags with larger place come first. (NULLs last)
    return executeQuery("SELECT name FROM tag "
                        "WHERE categoryId=%s ORDER BY place DESC",
                        Bindings() << categoryId).asStringList();
}

QStringList QueryHelper::folders() const
{
    return executeQuery("SELECT path FROM dir").asStringList();
}

uint QueryHelper::mediaItemCount(DB::MediaType typemask,
                                 QValueList<int>* scope) const
{
    if (!scope) {
        if (typemask == DB::anyMediaType)
            return executeQuery("SELECT COUNT(*) FROM media"
                                ).firstItem().toUInt();
        else
            return executeQuery("SELECT COUNT(*) FROM media WHERE " +
                                typeCondition("type", typemask)
                                ).firstItem().toUInt();
    }
    else {
        if (scope->isEmpty())
            return 0; // Empty scope contains no media items
        else {
            if (typemask == DB::anyMediaType) {
                return executeQuery("SELECT COUNT(*) FROM media "
                                    "WHERE id IN (%s)",
                                    Bindings() << toVariantList(*scope)
                                    ).firstItem().toUInt();
            }
            else {
                return executeQuery("SELECT COUNT(*) FROM media WHERE " +
                                    typeCondition("type", typemask) +
                                    " AND id IN (%s)",
                                    Bindings() << toVariantList(*scope)
                                    ).firstItem().toUInt();
            }
        }
    }
}

QValueList<int> QueryHelper::mediaItemIds(DB::MediaType typemask) const
{
    if (typemask == DB::anyMediaType)
        return executeQuery("SELECT id FROM media ORDER BY place"
                            ).asIntegerList();
    else
        return executeQuery("SELECT id FROM media "
                            "WHERE " + typeCondition("type", typemask) +
                            " ORDER BY place").asIntegerList();
}

void QueryHelper::getMediaItem(int id, DB::ImageInfo& info) const
{
    RowData row;
    try {
        row =
            executeQuery("SELECT dir.path, m.filename, m.md5sum, m.type, "
                         "m.label, m.description, "
                         "m.startTime, m.endTime, m.width, m.height, m.angle "
                         "FROM media m, dir "
                         "WHERE m.dirId=dir.id AND "
                         "m.id=%s", Bindings() << id).getRow();
    }
    catch (RowNotFoundError&) {
        throw EntryNotFoundError();
    }

    Q_ASSERT(row.count() == 11);

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
        info.addCategoryInfo((*i)[0], (*i)[1]);

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
    executeStatement("DELETE FROM membergroup "
                     "WHERE groupTag=%s OR memberTag=%s",
                     Bindings() << tagId << tagId);
#endif
    executeStatement("DELETE FROM tag WHERE categoryId=%s AND name=%s",
                     Bindings() << categoryId << name);
}

void QueryHelper::insertMediaTag(int mediaId, int tagId)
{
    if (executeQuery("SELECT COUNT(*) FROM media_tag "
                     "WHERE mediaId=%s AND tagId=%s",
                     Bindings() << mediaId << tagId).firstItem().toUInt() > 0)
        return;
    executeStatement("INSERT INTO media_tag (mediaId, tagId) "
                     "VALUES (%s, %s)", Bindings() << mediaId << tagId);
}

void QueryHelper::insertMediaItemTags(int mediaId, const DB::ImageInfo& info)
{
    QStringList categories = info.availableCategories();
    for (QStringList::const_iterator categoryIt = categories.begin();
         categoryIt != categories.end(); ++categoryIt) {
        if (*categoryIt == "Folder")
            continue;
        int cid = categoryId(*categoryIt);
        StringSet items = info.itemsOfCategory(*categoryIt);
        for (StringSet::const_iterator itemIt = items.begin();
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

int QueryHelper::insertDir(const QString& dirname)
{
    QVariant i = executeQuery("SELECT id FROM dir "
                              "WHERE path=%s",
                              Bindings() << dirname).firstItem();
    if (!i.isNull())
        return i.toInt();

    return insert("dir", "id", QStringList() << "path",
                  Bindings() << dirname);
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
    Bindings infoValues = imageInfoToBindings(info);

    if (executeQuery("SELECT COUNT(*) FROM media "
                     "WHERE dirId=%s AND filename=%s",
                     Bindings() << infoValues[0] << infoValues[1]
                     ).firstItem().toUInt() != 0)
        return;

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
    bindings += infoValues;

    Q_ULLONG mediaId = insert("media", "id", fields, bindings);

    insertMediaItemTags(mediaId, info);
    insertMediaItemDrawings(mediaId, info);
}

void
QueryHelper::insertMediaItemsLast(const QValueList<DB::ImageInfoPtr>& items)
{
    KexiDB::TransactionGuard transaction(*_connection);

    int place =
        executeQuery("SELECT MAX(place) FROM media").firstItem().toInt() + 1;

    for (QValueList<DB::ImageInfoPtr>::const_iterator i = items.constBegin();
         i != items.constEnd(); ++i) {
        insertMediaItem(*(*i), place++);
    }

    transaction.commit();
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

QValueList<int> QueryHelper::directMembers(int tagId) const
{
    return executeQuery("SELECT memberTag FROM membergroup WHERE groupTag=%s",
                        Bindings() << tagId).asIntegerList();
}

int QueryHelper::tagId(const QString& category, const QString& item) const
{
    QVariant id =
        executeQuery("SELECT tag.id FROM tag,category "
                     "WHERE tag.categoryId=category.id AND "
                     "category.name=%s AND tag.name=%s",
                     Bindings() << category << item).firstItem();
    if (id.isNull())
        throw EntryNotFoundError();
    else
        return id.toInt();
}

QValueList<int> QueryHelper::tagIdList(const QString& category,
                                       const QString& item) const
{
    int rootTagId = tagId(category, item);
    QValueList<int> visited;
    QValueList<int> queue;
    visited << rootTagId;
    queue << rootTagId;
    while (!queue.empty()) {
        QValueList<int> adj = directMembers(queue.first());
        queue.pop_front();
        QValueListConstIterator<int> adjEnd(adj.constEnd());
        for (QValueList<int>::const_iterator a = adj.constBegin();
             a != adjEnd; ++a) {
            if (!visited.contains(*a)) { // FIXME: Slow to find
                queue << *a;
                visited << *a;
            }
        }
    }
    return visited;
}

QValueList<QString[3]> QueryHelper::memberGroupConfiguration() const
{
    return executeQuery("SELECT c.name, g.name, m.name "
                        "FROM membergroup mg, tag g, tag m, category c "
                        "WHERE mg.groupTag=g.id AND "
                        "mg.memberTag=m.id AND "
                        "g.categoryId=c.id").asString3List();
}

QValueList<QString[2]>
QueryHelper::memberGroupConfiguration(const QString& category) const
{
    return executeQuery("SELECT g.name, m.name "
                        "FROM membergroup mg, tag g, tag m, category c "
                        "WHERE mg.groupTag=g.id AND "
                        "mg.memberTag=m.id AND "
                        "g.categoryId=c.id AND c.name=%s",
                        Bindings() << category).asString2List();
}

void QueryHelper::addBlockItem(const QString& filename)
{
    QString path;
    QString basename;
    splitPath(filename, path, basename);
    int dirId = insertDir(path);
    if (executeQuery("SELECT COUNT(*) FROM blockitem "
                     "WHERE dirId=%s AND filename=%s", Bindings() <<
                     dirId << basename).firstItem().asUInt() == 0)
        executeStatement("INSERT INTO blockitem (dirId, filename) "
                         "VALUES (%s, %s)", Bindings() << dirId << basename);
}

void QueryHelper::addBlockItems(const QStringList& filenames)
{
    for (QStringList::const_iterator i = filenames.begin();
         i != filenames.end(); ++i)
        addBlockItem(*i);
}

bool QueryHelper::isBlocked(const QString& filename) const
{
    QString path;
    QString basename;
    splitPath(filename, path, basename);
    return executeQuery("SELECT COUNT(*) FROM blockitem, dir "
                        "WHERE blockitem.dirId=dir.id AND "
                        "dir.path=%s AND blockitem.filename=%s",
                        Bindings() << path << basename
                        ).firstItem().toUInt() > 0;
}

void QueryHelper::removeMediaItem(const QString& filename)
{
    int id = mediaItemId(filename);
    executeStatement("DELETE FROM media_tag WHERE mediaId=%s",
                     Bindings() << id);
    executeStatement("DELETE FROM media WHERE id=%s", Bindings() << id);
}

void QueryHelper::insertCategory(const QString& name,
                                 const QString& icon, bool visible,
                                 DB::Category::ViewType type,
                                 int thumbnailSize)
{
    executeStatement("INSERT INTO category (name, icon, visible, "
                     "viewtype, thumbsize) VALUES (%s, %s, %s, %s, %s)",
                     Bindings() << name << icon <<
                     QVariant(visible, 0 /* dummy, to make it bool */) <<
                     type << thumbnailSize);
}

void QueryHelper::removeCategory(const QString& name)
{
    int id = categoryId(name);
    Cursor c = executeQuery("SELECT id FROM tag WHERE categoryId=%s").cursor();
    for (c.selectFirstRow(); c.rowExists(); c.selectNextRow()) {
        QVariant tagId = c.value(0);
        executeStatement("DELETE FROM media_tag WHERE tagId=%s",
                         Bindings() << tagId);
        executeStatement("DELETE FROM membergroup "
                         "WHERE groupTag=%s OR memberTag=%s",
                         Bindings() << tagId << tagId);
    }
    executeStatement("DELETE FROM tag WHERE categoryId=%s", Bindings() << id);
    executeStatement("DELETE FROM category WHERE id=%s", Bindings() << id);
}

QString QueryHelper::categoryName(int id) const
{
    QVariant r = executeQuery("SELECT name FROM category WHERE id=%s",
                              Bindings() << id).firstItem();

    if (r.isNull())
        throw EntryNotFoundError();
    else
        return r.toString();
}

QString QueryHelper::categoryIcon(int id) const
{
    return executeQuery("SELECT icon FROM category WHERE id=%s",
                        Bindings() << id).firstItem().toString();
}

bool QueryHelper::categoryVisible(int id) const
{
    return executeQuery("SELECT visible FROM category WHERE id=%s",
                        Bindings() << id).firstItem().toBool();
}

DB::Category::ViewType QueryHelper::categoryViewType(int id) const
{
    return static_cast<DB::Category::ViewType>
        (executeQuery("SELECT viewtype FROM category WHERE id=%s",
                      Bindings() << id).firstItem().toUInt());
}

void QueryHelper::changeCategoryName(int id, const QString& newName)
{
    executeStatement("UPDATE category SET name=%s WHERE id=%s",
                     Bindings() << newName << id);
}

void QueryHelper::changeCategoryIcon(int id, const QString& icon)
{
    executeStatement("UPDATE category SET icon=%s WHERE id=%s",
                     Bindings() << icon << id);
}

void QueryHelper::changeCategoryVisible(int id, bool visible)
{
    executeStatement("UPDATE category SET visible=%s WHERE id=%s",
                     Bindings() << QVariant(visible, 0) << id);
}

void QueryHelper::changeCategoryViewType(int id, DB::Category::ViewType type)
{
    executeStatement("UPDATE category SET viewtype=%s WHERE id=%s",
                     Bindings() << type << id);
}

bool QueryHelper::containsMD5Sum(const QString& md5sum) const
{
    return executeQuery("SELECT COUNT(*) FROM media WHERE md5sum=%s",
                        Bindings() << md5sum).firstItem().toUInt() > 0;
}

QString QueryHelper::filenameForMD5Sum(const QString& md5sum) const
{
    QValueList<QString[2]> rows =
        executeQuery("SELECT dir.path, media.filename FROM dir, media "
                     "WHERE dir.id=media.dirId AND media.md5sum=%s",
                     Bindings() << md5sum).asString2List();
    if (rows.isEmpty())
        return QString::null;
    else {
        return makeFullName(rows[0][0], rows[0][1]);
    }
}

QValueList< QPair<int, QString> >
QueryHelper::mediaIdTagPairs(const QString& category,
                             DB::MediaType typemask) const
{
    if (category == "Folder")
        return executeQuery("SELECT media.id, dir.path FROM media, dir "
                            "WHERE media.dirId=dir.id AND " +
                            typeCondition("media.type", typemask)
                            ).asIntegerStringPairs();
    else
        return executeQuery("SELECT media.id, tag.name "
                            "FROM media, media_tag, tag, category "
                            "WHERE media.id=media_tag.mediaId AND "
                            "media_tag.tagId=tag.id AND "
                            "tag.categoryId=category.id AND " +
                            typeCondition("media.type", typemask) +
                            " AND category.name=%s",
                            Bindings() << category).asIntegerStringPairs();
}

int QueryHelper::mediaPlaceByFilename(const QString& filename) const
{
    QString path;
    QString basename;
    splitPath(filename, path, basename);
    QVariant place =
        executeQuery("SELECT media.place FROM media, dir "
                     "WHERE media.dirId=dir.id AND "
                     "dir.path=%s AND media.filename=%s",
                     Bindings() << path << basename).firstItem();
    if (place.isNull())
        throw EntryNotFoundError();
    return place.toInt();
}

/** Move media items after or before destination item.
 */
void QueryHelper::moveMediaItems(const QStringList& filenames,
                                 const QString& destinationFilename, bool after)
{

    // BROKEN!
    // TODO: make this function work!


    if (filenames.isEmpty())
        return;

    QValueList<int> srcIds;
    for (QStringList::const_iterator i = filenames.constBegin();
         i != filenames.constEnd(); ++i) {
        srcIds << mediaItemId(*i);
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
        mediaPlaceByFilename(destinationFilename) + (after ? 1 : 0);

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
                     "WHERE b.place<a.place OR (b.place=a.place AND b.id<a.id)"
                     "GROUP BY a.id) x "
                     "SET media.place=x.n WHERE media.id=x.id");
}

void QueryHelper::sortMediaItems(const QStringList& filenames)
{
    KexiDB::TransactionGuard transaction(*_connection);

    QValueList<int> idList = mediaItemIdsForFilenames(filenames);

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
                                              bool includeRanges) const
{
    return findFirstFileInTimeRange(range, includeRanges, 0);
}

QString
QueryHelper::findFirstFileInTimeRange(const DB::ImageDate& range,
                                      bool includeRanges,
                                      const QValueList<int>& idList) const
{
    return findFirstFileInTimeRange(range, includeRanges, &idList);
}

QString
QueryHelper::findFirstFileInTimeRange(const DB::ImageDate& range,
                                      bool includeRanges,
                                      const QValueList<int>* idList) const
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

namespace
{
    void split(const MatcherList& input,
               MatcherList& positiveList, MatcherList& negativeList)
    {
        for (MatcherList::const_iterator it = input.constBegin();
             it != input.constEnd(); ++it) {
            DB::OptionValueMatcher* valueMatcher;
            valueMatcher = dynamic_cast<DB::OptionValueMatcher*>(*it);
            if (valueMatcher) {
                if (valueMatcher->_sign)
                    positiveList.append(valueMatcher);
                else
                    negativeList.append(valueMatcher);
            }
            else
                negativeList.append(*it);
        }
    }
}

QValueList<int>
QueryHelper::searchMediaItems(const DB::ImageSearchInfo& search,
                              DB::MediaType typemask) const
{
    MatcherListList dnf = search.query();
    // dnf is in Disjunctive Normal Form ( OR(AND(a,b),AND(c,d)) )

    if (dnf.isEmpty())
        return mediaItemIds(typemask);

    QValueList<int> r;
    for (MatcherListList::const_iterator i = dnf.begin();
         i != dnf.end(); ++i) {
         r = mergeListsUniqly(r, getMatchingFiles(*i, typemask));
    }

    return r;
}

QValueList<int>
QueryHelper::getMatchingFiles(MatcherList matches,
                              DB::MediaType typemask) const
{
    MatcherList positiveList;
    MatcherList negativeList;
    split(matches, positiveList, negativeList);

    /*
    SELECT id FROM media
    WHERE
    id IN (SELECT mediaId FROM media_tag WHERE tagId IN (memberItem1TagIds))
    AND
    id IN (SELECT mediaId FROM media_tag WHERE tagId IN (memberItem2TagIds))
    AND ...
    */

    // Positive part of the query
    QStringList positiveQuery;
    QMap<QString, QValueList<int> > matchedTags;
    QStringList matchedFolders;
    Bindings binds;
    for (MatcherList::const_iterator i = positiveList.begin();
         i != positiveList.end(); ++i) {
        DB::OptionValueMatcher* m = static_cast<DB::OptionValueMatcher*>(*i);
        if (m->_category == "Folder") {
            positiveQuery <<
                "id IN (SELECT media.id FROM media, dir "
                "WHERE media.dirId=dir.id AND dir.path=%s)";
            binds << m->_option;
            matchedFolders += m->_option;
        }
        else {
            positiveQuery <<
                "id IN (SELECT mediaId FROM media_tag WHERE tagId IN (%s))";
            QValueList<int> tagIds = tagIdList(m->_category, m->_option);
            binds << toVariantList(tagIds);
            matchedTags[m->_category] += tagIds;
        }
    }

    // Negative query
    QStringList negativeQuery;
    QStringList excludeQuery;
    Bindings excBinds;
    for (MatcherList::const_iterator i = negativeList.begin();
         i != negativeList.end(); ++i) {
        DB::OptionValueMatcher* m = dynamic_cast<DB::OptionValueMatcher*>(*i);
        if (m) {
            if (m->_category == "Folder") {
                negativeQuery <<
                    "id NOT IN (SELECT media.id FROM media, dir "
                    "WHERE media.dirId=dir.id AND dir.path=%s)";
                binds << m->_option;
            }
            else {
                negativeQuery <<
                    "id NOT IN (SELECT mediaId "
                    "FROM media_tag WHERE tagId IN (%s))";
                binds << toVariantList(tagIdList(m->_category, m->_option));
            }
        }
        else {
            if ((*i)->_category == "Folder") {
                QStringList excludedFolders;
                if (!matchedFolders.isEmpty()) {
                    excludedFolders = matchedFolders;
                }
                else {
                    excludedFolders = executeQuery("SELECT path FROM dir").
                        asStringList();
                }
                if (!excludedFolders.isEmpty()) {
                    excludeQuery <<
                        "id IN (SELECT media.id FROM media, dir "
                        "WHERE media.dirId=dir.id AND dir.path IN (%s))";
                    excBinds << toVariantList(excludedFolders);
                }
            }
            else {
                QValueList<int> excludedTags;
                if (!matchedTags[(*i)->_category].isEmpty()) {
                    excludedTags = matchedTags[(*i)->_category];
                } else {
                    excludedTags = tagIdsOfCategory((*i)->_category);
                }
                if (!excludedTags.isEmpty()) {
                    excludeQuery <<
                        "id IN (SELECT mediaId "
                        "FROM media_tag WHERE tagId IN (%s))";
                    excBinds << toVariantList(excludedTags);
                }
            }
        }
    }

    QString select = "SELECT id FROM media";
    QStringList condList = positiveQuery + negativeQuery;

    if (typemask != DB::anyMediaType)
        condList.prepend(typeCondition("type", typemask));

    QString cond = condList.join(" AND ");

    QString query = select;
    if (!cond.isEmpty())
        query += " WHERE " + cond;

    query += " ORDER BY place";

    QValueList<int> positive = executeQuery(query, binds).asIntegerList();

    if (excludeQuery.isEmpty())
        return positive;

    QValueList<int> negative =
        executeQuery(select + " WHERE " + excludeQuery.join(" OR "),
                     excBinds).asIntegerList();

    return listSubtract(positive, negative);
}

// TODO: remove dependencies to these two
#include "DB/ImageDB.h"
#include "DB/GroupCounter.h"


QMap<QString, uint>
QueryHelper::classify(const QString& category,
                      DB::MediaType typemask,
                      QValueList<int>* scope) const
{
    QMap<QString, uint> result;
    DB::GroupCounter counter( category );

    QValueList< QPair<int, QString> > idTagPairs =
        mediaIdTagPairs(category, typemask);

    QMap<int,QStringList> itemMap;
    for (QValueList< QPair<int, QString> >::const_iterator
             i = idTagPairs.begin(); i != idTagPairs.end(); ++i) {
        int fileId = (*i).first;
        QString item = (*i).second;
        if (!scope || scope->contains(fileId))
            itemMap[fileId].append(item);
    }

    // Count images that doesn't contain an item
    if (!scope)
        result[DB::ImageDB::NONE()] = mediaItemCount(typemask) - itemMap.count();
    else
        result[DB::ImageDB::NONE()] = scope->count() - itemMap.count();

    for( QMap<int,QStringList>::Iterator mapIt = itemMap.begin(); mapIt != itemMap.end(); ++mapIt ) {
        QStringList list = mapIt.data();
        for( QStringList::Iterator listIt = list.begin(); listIt != list.end(); ++listIt ) {
            //if ( !alreadyMatched[ *listIt ] ) // We do not want to match "Jesper & Jesper"
                result[ *listIt ]++;
        }
        counter.count( list );
    }

    QMap<QString,uint> groups = counter.result();
    for( QMapIterator<QString,uint> it= groups.begin(); it != groups.end(); ++it ) {
        result[it.key()] = it.data();
    }
    return result;
}
