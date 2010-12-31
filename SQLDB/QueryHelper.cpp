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

#include "QueryHelper.h"
#include <DB/ValueCategoryMatcher.h>
#include "QueryErrors.h"
#include "SQLImageInfo.h"
#include "DB/ImageSearchInfo.h"
#include "Utilities/List.h"
#include "Utilities/QStr.h"
#include "TransactionGuard.h"
#include <klocale.h>
#include <qsize.h>
#include <QList>
#include <QSet>
#include <QSqlField>
#include <QSqlDriver>
#include <QSqlError>
#include <memory>
#include <QtDebug>

using namespace SQLDB;
using Utilities::mergeListsUniqly;
using Utilities::listSubtract;

typedef QPair<QString, QString> StringPair;

namespace
{
void splitPath(const QString& filename, QString& path, QString& basename)
{
    int i = filename.lastIndexOf(QString::fromLatin1("/"));
    if (i == -1) {
        path = QString::fromLatin1(".");
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
    if (path == QString::fromLatin1("."))
        return basename;
    else
        return path + QString::fromLatin1("/") + basename;
}

QString typeCondition(const QString& fieldName, DB::MediaType typemask)
{
    if (typemask == DB::Image ||
        typemask == DB::Video)
        return fieldName + QString::fromLatin1("=") + QString::number(typemask);
    else if (typemask == DB::anyMediaType)
        return QString::fromLatin1("1=1");
    else {
        QStringList typeList;
        for (uint t = 1; t < DB::anyMediaType; t <<= 1)
            if (typemask & t)
                typeList.append(fieldName + QString::fromLatin1("=") + QString::number(t));
        return QString::fromLatin1("(") + typeList.join(QString::fromLatin1(" OR ")) + QString::fromLatin1(")");
    }
}
}

QStringList QueryHelper::filenames() const
{
    QList<StringPair> dirFilePairs =
        executeQuery(
            "SELECT directory.path, file.filename"
            " FROM directory, file"
            " WHERE file.directory_id=directory.id"
            " ORDER BY file.position"
            ).asList<StringPair>();
    QStringList r;
    for (QList<StringPair>::const_iterator i = dirFilePairs.constBegin();
         i != dirFilePairs.constEnd(); ++i) {
        r << makeFullName((*i).first, (*i).second);
    }
    return r;
}

QString QueryHelper::mediaItemFilename(DB::RawId id) const
{
    QList<StringPair> dirFilePairs =
        executeQuery(
            "SELECT directory.path, file.filename"
            " FROM directory, file"
            " WHERE directory.id=file.directory_id"
            " AND file.id=?",
            Bindings() << id).asList<StringPair>();
    if (dirFilePairs.isEmpty())
        throw EntryNotFoundError();

    return makeFullName(dirFilePairs[0].first, dirFilePairs[0].second);
}

DB::RawId QueryHelper::mediaItemId(const QString& filename) const
{
    QString path;
    QString basename;
    splitPath(filename, path, basename);
    QVariant id =
        executeQuery(
            "SELECT file.id FROM file, directory"
            " WHERE file.directory_id=directory.id"
            " AND directory.path=? AND file.filename=?",
        Bindings() << path << basename).firstItem();
    if (id.isNull())
        throw EntryNotFoundError();
    Q_ASSERT(id.toInt() > 0);
    return DB::RawId(id.toInt());
}

QList< QPair<DB::RawId, QString> > QueryHelper::mediaItemIdFileMap() const
{
    Cursor c = executeQuery(
        "SELECT file.id, directory.path, file.filename"
        " FROM file, directory"
        " WHERE file.directory_id=directory.id").cursor();
    QList< QPair<DB::RawId, QString> > r;

    if (c.isNull())
        return r;

    for (c.selectFirstRow(); c.rowExists(); c.selectNextRow()) {
        QPair<DB::RawId, QString> x;
        x.first = DB::RawId(c.value(0).toInt());
        Q_ASSERT(x.first != DB::RawId());
        x.second = makeFullName(c.value(1).toString(), c.value(2).toString());
        r << x;
    }
    return r;
}

QList<DB::RawId>
QueryHelper::mediaItemIdsForFilenames(const QStringList& filenames) const
{
#if 1
    QStringList paths;
    QStringList basenames;
    QMap<QString, DB::RawId> pathIdMap;
    for (QStringList::const_iterator i = filenames.constBegin();
         i != filenames.constEnd(); ++i) {
        QString path;
        QString basename;
        splitPath(*i, path, basename);
        paths << path;
        basenames << basename;
        if (!pathIdMap.contains(path)) {
            const DB::RawId id(
                executeQuery(
                    "SELECT id FROM directory"
                    " WHERE path=?",
                    Bindings() << path
                    ).firstItem().toInt());
            Q_ASSERT(id != DB::RawId());
            pathIdMap.insert(path, id);
        }
    }
    QList<DB::RawId> idList;
    QStringList::const_iterator pathIt = paths.constBegin();
    QStringList::const_iterator basenameIt = basenames.constBegin();
    while (pathIt != paths.constEnd()) {
        const QVariant id =
            executeQuery(
                "SELECT id"
                " FROM file"
                " WHERE directory_id=? AND filename=?",
            Bindings() << pathIdMap[*pathIt] << *basenameIt
            ).firstItem();
        /*
        if (id.isNull())
            throw EntryNotFoundError();
        */
        if (!id.isNull())
            idList << DB::RawId(id.toInt());
        Q_ASSERT(idList[idList.size() - 1] != DB::RawId());
        ++pathIt;
        ++basenameIt;
    }
    return idList;
#else
    // Uses CONCAT function, which is MySQL specific.

    QStringList adjustedPaths = filenames;
    for (QStringList::iterator i = adjustedPaths.constBegin(); i != adjustedPaths.constEnd(); ++i) {
        if ((*i).find('/') == -1) {
            *i = QString::fromLatin1("./") + *i;
        }
    }
    executeQuery("SELECT id"
                 " FROM file, directory"
                 " WHERE CONCAT(directory.path, '/', file.filename) IN (?)");
#endif
}

QStringList QueryHelper::categoryNames() const
{
    return executeQuery("SELECT name FROM category").asList<QString>();
}

int QueryHelper::categoryId(const QString& category) const
{
    QVariant r = executeQuery(
        "SELECT id"
        " FROM category"
        " WHERE name=?",
        Bindings() << category).firstItem();
    if (r.isNull())
        throw EntryNotFoundError();
    else
        return r.toInt();
}

QList<int> QueryHelper::tagIdsOfCategory(const QString& category) const
{
    return executeQuery(
        "SELECT tag.id FROM tag, category"
        " WHERE tag.category_id=category.id"
        " AND category.name=?",
        Bindings() << category).asList<int>();
}

QStringList QueryHelper::tagNamesOfCategory(int categoryId) const
{
    // Tags with larger position come first. (NULLs last)
    return executeQuery(
        "SELECT name FROM tag"
        " WHERE category_id=?"
        " ORDER BY position DESC",
        Bindings() << categoryId).asList<QString>();
}

QStringList QueryHelper::folders() const
{
    return executeQuery("SELECT path FROM directory").asList<QString>();
}

uint QueryHelper::mediaItemCount(DB::MediaType typemask,
                                 QList<DB::RawId>* scope) const
{
    if (!scope) {
        if (typemask == DB::anyMediaType)
            return executeQuery(
                "SELECT COUNT(*) FROM file").firstItem().toUInt();
        else
            return executeQuery(
                (QStr("SELECT COUNT(*) FROM file WHERE ") +
                 typeCondition(QStr("type"), typemask)).toAscii().constData()
                ).firstItem().toUInt();
    }
    else {
        if (scope->isEmpty())
            return 0; // Empty scope contains no media items
        else {
            if (typemask == DB::anyMediaType) {
                return executeQuery(
                    "SELECT COUNT(*) FROM file"
                    " WHERE id IN (?)",
                    Bindings() << QVariant(toVariantList(*scope))
                    ).firstItem().toUInt();
            }
            else {
                return executeQuery(
                    (QStr("SELECT COUNT(*) FROM file WHERE ") +
                     typeCondition(QStr("type"), typemask) +
                     QStr(" AND id IN (?)")).toAscii().constData(),
                    Bindings() << QVariant(toVariantList(*scope))
                    ).firstItem().toUInt();
            }
        }
    }
}

QList<DB::RawId> QueryHelper::mediaItemIds(DB::MediaType typemask) const
{
    if (typemask == DB::anyMediaType)
        return executeQuery(
            "SELECT id FROM file ORDER BY position").asList<DB::RawId>();
    else
        return executeQuery(
            (QStr("SELECT id FROM file WHERE ") +
             typeCondition(QStr("type"), typemask) +
             QStr(" ORDER BY position")).toAscii().constData()
            ).asList<DB::RawId>();
}

void QueryHelper::getMediaItem(DB::RawId id, DB::ImageInfo& info) const
{
    qDebug() << "Getting image info of id" << id;
    const QMap<DB::RawId, DB::ImageInfoPtr> infos(getInfosOfFiles(QList<DB::RawId>() << id));
    if (!infos.isEmpty())
        info = *infos[id];
    else
        throw EntryNotFoundError();
}

int QueryHelper::insertTag(int categoryId, const QString& name)
{
    QVariant i = executeQuery(
        "SELECT id FROM tag"
        " WHERE category_id=? AND name=?",
        Bindings() << categoryId << name).firstItem();
    if (!i.isNull())
        return i.toInt();

    return executeInsert(
        QStr("tag"), QStr("id"),
        QStringList() << QStr("category_id") << QStr("name"),
        Bindings() << categoryId << name);
}

void QueryHelper::insertTagFirst(int categoryId, const QString& name)
{
    // See membersOfCategory() for tag ordering usage.

    int id = insertTag(categoryId, name);
    QVariant oldPosition = executeQuery(
        "SELECT position FROM tag WHERE id=?",
        Bindings() << id).firstItem();

    // Move tags from this to previous first tag one position towards end
    if (!oldPosition.isNull()) {
        executeStatement(
            "UPDATE tag SET position=position-1"
            " WHERE category_id=? AND position>?",
            Bindings() << categoryId << oldPosition);
    }

    // MAX(position) could be NULL, but it'll be returned as 0 with
    // toInt(), which is ok.
    int newPosition = executeQuery(
        "SELECT MAX(position) FROM tag"
        " WHERE category_id=?",
        Bindings() << categoryId
        ).firstItem().toInt() + 1;

    // Put this tag first
    executeStatement("UPDATE tag SET position=? WHERE id=?",
                     Bindings() << newPosition << id);
}

void QueryHelper::removeTag(int categoryId, const QString& name)
{
#ifndef USEFOREIGNCONSTRAINT
    int tagId =
        executeQuery("SELECT id FROM tag"
                     " WHERE category_id=? AND name=?",
                     Bindings() << categoryId << name).firstItem().toInt();
    executeStatement("DELETE FROM file_tag WHERE tag_id=?",
                     Bindings() << tagId);
    executeStatement("DELETE FROM tag_member "
                     "WHERE tag_id=? OR member_tag_id=?",
                     Bindings() << tagId << tagId);
#endif
    executeStatement("DELETE FROM tag WHERE category_id=? AND name=?",
                     Bindings() << categoryId << name);
}

void QueryHelper::insertMediaTag(DB::RawId mediaId, int tagId)
{
    if (executeQuery("SELECT COUNT(*) FROM file_tag"
                     " WHERE file_id=? AND tag_id=?",
                     Bindings() << mediaId << tagId).firstItem().toUInt() > 0)
        return;
    executeStatement("INSERT INTO file_tag (file_id, tag_id)"
                     " VALUES (?, ?)", Bindings() << mediaId << tagId);
}

void QueryHelper::insertMediaItemTags(DB::RawId mediaId, const DB::ImageInfo& info)
{
    QStringList categories = info.availableCategories();
    for (QStringList::const_iterator categoryIt = categories.constBegin();
         categoryIt != categories.constEnd(); ++categoryIt) {
        if (*categoryIt == QStr("Folder"))
            continue;
        int cid = categoryId(*categoryIt);
        StringSet items = info.itemsOfCategory(*categoryIt);
        for (StringSet::const_iterator itemIt = items.constBegin();
             itemIt != items.constEnd(); ++itemIt) {
            qulonglong tagId = insertTag(cid, *itemIt);
            insertMediaTag(mediaId, tagId);
        }
    }
}

int QueryHelper::insertDir(const QString& dirname)
{
    QVariant i = executeQuery("SELECT id FROM directory"
                              " WHERE path=?",
                              Bindings() << dirname).firstItem();
    if (!i.isNull())
        return i.toInt();

    return executeInsert(
        QStr("directory"), QStr("id"), QStringList() << QStr("path"),
        Bindings() << dirname);
}

/** Get data from ImageInfo to Bindings list.
 *
 * Adds also image directory to database, if it's not already there.
 *
 * @return Bingings in following order:
 *  directory_id, filename, md5sum, type, label, description,
 *  time_start, time_end, width, height, angle
 */
QueryHelper::Bindings
QueryHelper::imageInfoToBindings(const DB::ImageInfo& info)
{
    QVariant md5;
    if (!info.MD5Sum().isNull())
        md5 = info.MD5Sum().toHexString();
    QVariant w = info.size().width();
    if (w.toInt() == -1)
        w = QVariant();
    QVariant h =  info.size().height();
    if (h.toInt() == -1)
        h = QVariant();
    QString path;
    QString filename;
    splitPath(info.fileName(DB::RelativeToImageRoot), path, filename);
    QVariant rating;
    if (info.rating() == -1)
        rating = info.rating();
    QVariant stackId;
    QVariant stackPosition;
    if (info.stackId() != 0) {
        stackId = info.stackId();
        stackPosition = info.stackOrder();
    }
    QVariant gpsLongitude;
    QVariant gpsLatitude;
    QVariant gpsAltitude;
    QVariant gpsPrecision;
    {
        const DB::GpsCoordinates& geoPos = info.geoPosition();
        if (!geoPos.isNull()) {
            gpsLongitude = geoPos.longitude();
            gpsLatitude = geoPos.latitude();
            gpsAltitude = geoPos.altitude();
            if (geoPos.precision() != DB::GpsCoordinates::NoPrecisionData)
                gpsPrecision = geoPos.precision();
        }
    }

    int dirId = insertDir(path);

    return Bindings() <<
        dirId << filename << md5 <<
        info.mediaType() << info.label() <<
        info.description() <<
        info.date().start() << info.date().end() <<
        w << h << info.angle() <<
        rating << stackId << stackPosition <<
        gpsLongitude << gpsLatitude << gpsAltitude << gpsPrecision;
}

void QueryHelper::insertMediaItem(const DB::ImageInfo& info, int position)
{
    Bindings infoValues = imageInfoToBindings(info);

    if (executeQuery("SELECT COUNT(*) FROM file"
                     " WHERE directory_id=? AND filename=?",
                     Bindings() << infoValues[0] << infoValues[1]
                     ).firstItem().toUInt() != 0)
        return;

    // TODO: remove debug
    qDebug("Inserting info of file %s", info.fileName(DB::AbsolutePath).toLocal8Bit().data());

    QStringList fields;
    Bindings bindings;
    if (position != 0) {
        fields << QStr("position");
        bindings << position;
    }
    fields <<
        QStr("directory_id") << QStr("filename") << QStr("md5sum") <<
        QStr("type") << QStr("label") <<
        QStr("description") <<
        QStr("time_start") << QStr("time_end") <<
        QStr("width") << QStr("height") << QStr("angle") <<
        QStr("rating") << QStr("stack_id") << QStr("stack_position") <<
        QStr("gps_longitude") << QStr("gps_latitude") <<
        QStr("gps_altitude") << QStr("gps_precision");
    bindings += infoValues;

    const DB::RawId mediaId(
        executeInsert(QStr("file"), QStr("id"), fields, bindings));

    insertMediaItemTags(mediaId, info);
}

void
QueryHelper::insertMediaItemsLast(const QList<DB::ImageInfoPtr>& items)
{
    TransactionGuard transaction(*this);

    int position =
        executeQuery("SELECT MAX(position) FROM file").firstItem().toInt() + 1;

    for (QList<DB::ImageInfoPtr>::const_iterator i = items.constBegin();
         i != items.constEnd(); ++i) {
        insertMediaItem(*(*i), position++);
    }

    transaction.commit();
}

void QueryHelper::updateMediaItem(DB::RawId id, const DB::ImageInfo& info)
{
    // TODO: remove debug
    qDebug("Updating info of file %s", info.fileName(DB::AbsolutePath).toLocal8Bit().data());

    executeStatement("UPDATE file SET directory_id=?, filename=?, md5sum=?, "
                     "type""=?, label=?, description=?, "
                     "time_start""=?, time_end=?, "
                     "width=?, height=?, angle=?,"
                     " rating=?, stack_id=?, stack_position=?,"
                     " gps_longitude=?, gps_latitude=?,"
                     " gps_altitude=?, gps_precision=?"
                     " WHERE id=?",
                     imageInfoToBindings(info) << id);

    executeStatement("DELETE FROM file_tag WHERE file_id=?",
                     Bindings() << id);
    insertMediaItemTags(id, info);
}

QList<int> QueryHelper::directMembers(int tagId) const
{
    return executeQuery("SELECT member_tag_id FROM tag_member WHERE tag_id=?",
                        Bindings() << tagId).asList<int>();
}

int QueryHelper::tagId(const QString& category, const QString& item) const
{
    QVariant id =
        executeQuery("SELECT tag.id FROM tag, category"
                     " WHERE tag.category_id=category.id"
                     " AND category.name=? AND tag.name=?",
                     Bindings() << category << item).firstItem();
    if (id.isNull())
        throw EntryNotFoundError();
    else
        return id.toInt();
}

QList<int> QueryHelper::tagIdList(const QString& category,
                                       const QString& item) const
{
    int rootTagId = tagId(category, item);
    QSet<int> visited;
    QList<int> queue;
    visited << rootTagId;
    queue << rootTagId;
    while (!queue.empty()) {
        QList<int> adj = directMembers(queue.first());
        queue.pop_front();
        QList<int>::const_iterator adjEnd(adj.constEnd());
        for (QList<int>::const_iterator a = adj.constBegin();
             a != adjEnd; ++a) {
            if (!visited.contains(*a)) {
                queue << *a;
                visited << *a;
            }
        }
    }
    return visited.toList();
}

QList<StringPair>
QueryHelper::memberGroupConfiguration(const QString& category) const
{
    return executeQuery(
        "SELECT t.name, m.name"
        " FROM tag_member AS tm, tag AS t, tag AS m, category AS c"
        " WHERE tm.tag_id=t.id"
        " AND tm.member_tag_id=m.id"
        " AND t.category_id=c.id AND c.name=?",
        Bindings() << category).asList<StringPair>();
}

void QueryHelper::addIgnoredFile(const QString& filename)
{
    QString path;
    QString basename;
    splitPath(filename, path, basename);
    int dirId = insertDir(path);
    if (executeQuery("SELECT COUNT(*) FROM ignored_file"
                     " WHERE directory_id=? AND filename=?", Bindings() <<
                     dirId << basename).firstItem().toUInt() == 0)
        executeStatement("INSERT INTO ignored_file (directory_id, filename) "
                         "VALUES (?, ?)", Bindings() << dirId << basename);
}

void QueryHelper::addIgnoredFiles(const QStringList& filenames)
{
    for (QStringList::const_iterator i = filenames.constBegin();
         i != filenames.constEnd(); ++i)
        addIgnoredFile(*i);
}

bool QueryHelper::isIgnored(const QString& filename) const
{
    QString path;
    QString basename;
    splitPath(filename, path, basename);
    return executeQuery(
        "SELECT COUNT(*) FROM ignored_file, directory"
        " WHERE ignored_file.directory_id=directory.id"
        " AND directory.path=? AND ignored_file.filename=?",
        Bindings() << path << basename
        ).firstItem().toUInt() > 0;
}

void QueryHelper::removeMediaItem(DB::RawId id)
{
    executeStatement("DELETE FROM file_tag WHERE file_id=?",
                     Bindings() << id);
    executeStatement("DELETE FROM file WHERE id=?", Bindings() << id);
}

void QueryHelper::insertCategory(const QString& name,
                                 const QString& icon, bool visible,
                                 DB::Category::ViewType type,
                                 int thumbnailSize)
{
    executeStatement("INSERT INTO category (name, icon, visible, "
                     "viewtype, thumbsize) VALUES (?, ?, ?, ?, ?)",
                     Bindings() << name << icon <<
                     QVariant(visible) <<
                     type << thumbnailSize);
}

void QueryHelper::removeCategory(const QString& name)
{
    int id = categoryId(name);
    Cursor c = executeQuery("SELECT id FROM tag WHERE category_id=?").cursor();
    for (c.selectFirstRow(); c.rowExists(); c.selectNextRow()) {
        QVariant tagId = c.value(0);
        executeStatement("DELETE FROM file_tag WHERE tag_id=?",
                         Bindings() << tagId);
        executeStatement("DELETE FROM tag_member "
                         "WHERE tag_id=? OR member_tag_id=?",
                         Bindings() << tagId << tagId);
    }
    executeStatement("DELETE FROM tag WHERE category_id=?", Bindings() << id);
    executeStatement("DELETE FROM category WHERE id=?", Bindings() << id);
}

QString QueryHelper::categoryName(int id) const
{
    QVariant r = executeQuery("SELECT name FROM category WHERE id=?",
                              Bindings() << id).firstItem();

    if (r.isNull())
        throw EntryNotFoundError();
    else
        return r.toString();
}

QString QueryHelper::categoryIcon(int id) const
{
    return executeQuery("SELECT icon FROM category WHERE id=?",
                        Bindings() << id).firstItem().toString();
}

bool QueryHelper::categoryVisible(int id) const
{
    return executeQuery("SELECT visible FROM category WHERE id=?",
                        Bindings() << id).firstItem().toBool();
}

DB::Category::ViewType QueryHelper::categoryViewType(int id) const
{
    return static_cast<DB::Category::ViewType>
        (executeQuery("SELECT viewtype FROM category WHERE id=?",
                      Bindings() << id).firstItem().toUInt());
}

void QueryHelper::changeCategoryName(int id, const QString& newName)
{
    executeStatement("UPDATE category SET name=? WHERE id=?",
                     Bindings() << newName << id);
}

void QueryHelper::changeCategoryIcon(int id, const QString& icon)
{
    executeStatement("UPDATE category SET icon=? WHERE id=?",
                     Bindings() << icon << id);
}

void QueryHelper::changeCategoryVisible(int id, bool visible)
{
    executeStatement("UPDATE category SET visible=? WHERE id=?",
                     Bindings() << QVariant(visible) << id);
}

void QueryHelper::changeCategoryViewType(int id, DB::Category::ViewType type)
{
    executeStatement("UPDATE category SET viewtype=? WHERE id=?",
                     Bindings() << type << id);
}

bool QueryHelper::containsMD5Sum(const DB::MD5& md5sum) const
{
    return executeQuery("SELECT COUNT(*) FROM file WHERE md5sum=?",
                        Bindings() << md5sum.toHexString()
                        ).firstItem().toUInt() > 0;
}

QString QueryHelper::filenameForMD5Sum(const DB::MD5& md5sum) const
{
    QList<StringPair> rows =
        executeQuery("SELECT directory.path, file.filename FROM directory, file "
                     "WHERE directory.id=file.directory_id AND file.md5sum=?",
                     Bindings() << md5sum.toHexString()
                     ).asList<StringPair>();
    if (rows.isEmpty())
        return QString();
    else {
        return makeFullName(rows[0].first, rows[0].second);
    }
}

QMap<DB::RawId, StringSet>
QueryHelper::mediaIdTagsMap(const QString& category,
                            DB::MediaType typemask) const
{
    QString q;
    Bindings b;

    if (category == QStr("Folder")) {
        q = QStr(
            "SELECT file.id, directory.path FROM file, directory "
            "WHERE file.directory_id=directory.id AND ") +
            typeCondition(QStr("file.type"), typemask);
    }
    else {
        q = QStr(
            "SELECT file.id, tag.name "
            "FROM file, file_tag, tag, category "
            "WHERE file.id=file_tag.file_id AND "
            "file_tag.tag_id=tag.id AND "
            "tag.category_id=category.id AND ") +
            typeCondition(QStr("file.type"), typemask) +
            QStr(" AND category.name=?");
        b.append(category);
    }

    Cursor c = executeQuery(q.toAscii().constData(), b).cursor();

    QMap<DB::RawId, StringSet > r;
    if (!c.isNull()) {
        for (c.selectFirstRow(); c.rowExists(); c.selectNextRow()) {
            const DB::RawId id(c.value(0).toInt());
            r[id].insert(c.value(1).toString());
        }
    }
    return r;
}

int QueryHelper::getPositionOfFile(DB::RawId fileId) const
{
    QVariant position =
        executeQuery("SELECT position FROM file WHERE id=?",
                     Bindings() << fileId).firstItem();
    if (position.isNull())
        throw EntryNotFoundError();
    return position.toInt();
}

/** Move media items after or before destination item.
 */
void QueryHelper::moveMediaItems(
    const QList<DB::RawId>& srcIds,
    DB::RawId destinationFile,
    bool after)
{

    // BROKEN!
    // TODO: make this function work!


    if (srcIds.isEmpty())
        return;

    RowData minmax =
        executeQuery("SELECT MIN(position), MAX(position) "
                     "FROM file WHERE id IN (?)",
                     Bindings() << QVariant(toVariantList(srcIds))).getRow();
    int srcMin = minmax[0].toInt();
    int srcMax = minmax[1].toInt();

    // Move media items which are between srcIds just before first
    // item in srcIds list.
    QList<int> betweenList =
        executeQuery("SELECT id FROM file "
                     "WHERE ? <= position AND position <= ? AND id NOT IN (?)",
                     Bindings() << srcMin << srcMax << QVariant(toVariantList(srcIds))
                     ).asList<int>();
    int n = srcMin;
    for (QList<int>::const_iterator i = betweenList.constBegin();
         i != betweenList.constEnd(); ++i) {
        executeStatement("UPDATE file SET position=? WHERE id=?",
                         Bindings() << n << *i);
        ++n;
    }

    // Make items in srcIds continuous, so positions of items are
    // N, N+1, N+2, ..., N+n.
    for (QList<DB::RawId>::const_iterator i = srcIds.constBegin();
         i != srcIds.constEnd(); ++i) {
        executeStatement("UPDATE file SET position=? WHERE id=?",
                         Bindings() << n << *i);
        ++n;
    }

    int destPosition =
        getPositionOfFile(destinationFile) + (after ? 1 : 0);

    if (srcMin <= destPosition && destPosition <= srcMax)
        return;
    int low = destPosition <= srcMin ? destPosition : srcMin;
    int high = destPosition >= srcMax ? destPosition : srcMax;

    Q_ASSERT(low == destPosition || high == destPosition);

    uint N = srcIds.count();

    if (low < destPosition)
        executeStatement("UPDATE file SET position=position-? "
                         "WHERE ? <= position AND position <= ?",
                         Bindings() << N << low << destPosition - 1);
    else if (destPosition < high)
        executeStatement("UPDATE file SET position=position+? "
                         "WHERE ? <= position AND position <= ?",
                         Bindings() << N << destPosition << high - 1);

    executeStatement("UPDATE file SET position=position+(?) WHERE id IN (?)",
                     Bindings() << destPosition - srcMin << QVariant(toVariantList(srcIds)));
}

void QueryHelper::makeMediaPositionsContinuous()
{
    executeStatement("UPDATE file, "
                     "(SELECT a.id AS id, COUNT(*) AS n "
                     "FROM file a, file b "
                     "WHERE b.position<a.position OR (b.position=a.position AND b.id<a.id)"
                     "GROUP BY a.id) x "
                     "SET file.position=x.n WHERE file.id=x.id");
}

void QueryHelper::sortFiles(const QList<DB::RawId>& files)
{
    TransactionGuard transaction(*this);

    QList<DB::RawId> idList = files;

    executeStatement(
        "CREATE TEMPORARY TABLE idlist AS"
        " SELECT id FROM file WHERE id IN (?)",
        Bindings() << QVariant(toVariantList(files)));
    executeStatement(
        "CREATE TEMPORARY TABLE byposition AS"
        " SELECT a.position AS position, COUNT(*) AS n"
        " FROM file AS a, file AS b"
        " WHERE a.id IN (SELECT id FROM idlist)"
        " AND b.id IN (SELECT id FROM idlist)"
        " AND b.position <= a.position GROUP BY a.id");
    executeStatement(
        "CREATE TEMPORARY TABLE bytime AS"
        " SELECT a.id AS id, COUNT(*) AS n"
        " FROM file AS a, file AS b"
        " WHERE a.id IN (SELECT id FROM idlist)"
        " AND b.id IN (SELECT id FROM idlist)"
        " AND b.time_start <= a.time_start GROUP BY a.id");
#if 0
    executeStatement(
        "UPDATE file, byposition, bytime"
        " SET file.position=byposition.position"
        " WHERE file.id=bytime.id AND byposition.n=bytime.n");
#else
    Cursor c = executeQuery(
        "SELECT bytime.id, byposition.position"
        " FROM byposition JOIN bytime"
        " ON byposition.n=bytime.n").cursor();

    for (c.selectFirstRow(); c.rowExists(); c.selectNextRow()) {
        const int id = c.value(0).toInt();
        const int newPosition = c.value(1).toInt();
        executeStatement(
            "UPDATE file SET position=? WHERE id=?",
            Bindings() << newPosition << id);
    }
#endif
    executeStatement("DROP TABLE bytime");
    executeStatement("DROP TABLE byposition");
    executeStatement("DROP TABLE idlist");

    transaction.commit();
}

DB::Id QueryHelper::findFirstFileInTimeRange(
    const DB::ImageDate& range,
    bool includeRanges) const
{
    return findFirstFileInTimeRange(range, includeRanges, 0);
}

DB::Id
QueryHelper::findFirstFileInTimeRange(
    const DB::ImageDate& range,
    bool includeRanges,
    const QList<DB::RawId>& idList) const
{
    return findFirstFileInTimeRange(range, includeRanges, &idList);
}

DB::Id
QueryHelper::findFirstFileInTimeRange(
    const DB::ImageDate& range,
    bool includeRanges,
    const QList<DB::RawId>* idList) const
{
    QString query = QStr("SELECT id FROM file WHERE ");
    Bindings bindings;

    if (idList) {
        query += QStr("file.id IN (?) AND ");
        bindings << QVariant(toVariantList(*idList));
    }

    if (!includeRanges) {
        query += QStr("? <= file.time_start AND file.time_end <= ?");
    }
    else {
        query += QStr("? <= file.time_end AND file.time_start <= ?");
    }
    bindings << range.start() << range.end();

    query += QStr(" ORDER BY file.time_start LIMIT 1");

    const QList<DB::RawId> resultIds =
        executeQuery(query.toAscii().constData(), bindings).asList<DB::RawId>();

    if (resultIds.isEmpty())
        return DB::Id();
    else {
        return DB::Id::createContextless(resultIds.front());
    }
}

namespace
{
    void split(const MatcherList& input,
               MatcherList& positiveList, MatcherList& negativeList)
    {
        for (MatcherList::const_iterator it = input.constBegin();
             it != input.constEnd(); ++it) {
            DB::ValueCategoryMatcher* valueMatcher;
            valueMatcher = dynamic_cast<DB::ValueCategoryMatcher*>(*it);
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

QList<DB::RawId>
QueryHelper::searchMediaItems(const DB::ImageSearchInfo& search,
                              DB::MediaType typemask) const
{
    MatcherListList dnf(search.query());
    // dnf is in Disjunctive Normal Form ( OR(AND(a,b),AND(c,d)) )

    if (dnf.isEmpty())
        return mediaItemIds(typemask);

    QList<DB::RawId> r;
    for (MatcherListList::const_iterator i = dnf.constBegin();
         i != dnf.constEnd(); ++i) {
         r = mergeListsUniqly(r, getMatchingFiles(*i, typemask));
    }

    return r;
}

QList<DB::RawId>
QueryHelper::getMatchingFiles(MatcherList matches,
                              DB::MediaType typemask) const
{
    MatcherList positiveList;
    MatcherList negativeList;
    split(matches, positiveList, negativeList);

    /*
    SELECT id FROM file
    WHERE
    id IN (SELECT file_id FROM file_tag WHERE tag_id IN (memberItem1TagIds))
    AND
    id IN (SELECT file_id FROM file_tag WHERE tag_id IN (memberItem2TagIds))
    AND ...
    */

    // Positive part of the query
    QStringList positiveQuery;
    QMap<QString, QList<int> > matchedTags;
    QStringList matchedFolders;
    Bindings binds;
    for (MatcherList::const_iterator i = positiveList.constBegin();
         i != positiveList.constEnd(); ++i) {
        DB::ValueCategoryMatcher* m = static_cast<DB::ValueCategoryMatcher*>(*i);
        if (m->_category == QStr("Folder")) {
            positiveQuery << QStr(
                "id IN (SELECT file.id FROM file, directory "
                "WHERE file.directory_id=directory.id AND directory.path=?)");
            binds << m->_option;
            matchedFolders += m->_option;
        }
        else {
            positiveQuery << QStr(
                "id IN (SELECT file_id FROM file_tag WHERE tag_id IN (?))");
            QList<int> tagIds = tagIdList(m->_category, m->_option);
            binds << QVariant(toVariantList(tagIds));
            matchedTags[m->_category] += tagIds;
        }
    }

    // Negative query
    QStringList negativeQuery;
    QStringList excludeQuery;
    Bindings excBinds;
    for (MatcherList::const_iterator i = negativeList.constBegin();
         i != negativeList.constEnd(); ++i) {
        DB::ValueCategoryMatcher* m = dynamic_cast<DB::ValueCategoryMatcher*>(*i);
        if (m) {
            if (m->_category == QStr("Folder")) {
                negativeQuery << QStr(
                    "id NOT IN (SELECT file.id FROM file, directory "
                    "WHERE file.directory_id=directory.id AND directory.path=?)");
                binds << m->_option;
            }
            else {
                negativeQuery << QStr(
                    "id NOT IN (SELECT file_id "
                    "FROM file_tag WHERE tag_id IN (?))");
                binds << QVariant(toVariantList(tagIdList(m->_category, m->_option)));
            }
        }
        else {
            if ((*i)->_category == QStr("Folder")) {
                QStringList excludedFolders;
                if (!matchedFolders.isEmpty()) {
                    excludedFolders = matchedFolders;
                }
                else {
                    excludedFolders = executeQuery("SELECT path FROM directory").
                        asList<QString>();
                }
                if (!excludedFolders.isEmpty()) {
                    excludeQuery << QStr(
                        "id IN (SELECT file.id FROM file, directory "
                        "WHERE file.directory_id=directory.id AND directory.path IN (?))");
                    excBinds << QVariant(toVariantList(excludedFolders));
                }
            }
            else {
                QList<int> excludedTags;
                if (!matchedTags[(*i)->_category].isEmpty()) {
                    excludedTags = matchedTags[(*i)->_category];
                } else {
                    excludedTags = tagIdsOfCategory((*i)->_category);
                }
                if (!excludedTags.isEmpty()) {
                    excludeQuery << QStr(
                        "id IN (SELECT file_id "
                        "FROM file_tag WHERE tag_id IN (?))");
                    excBinds << QVariant(toVariantList(excludedTags));
                }
            }
        }
    }

    QString select = QStr("SELECT id FROM file");
    QStringList condList = positiveQuery + negativeQuery;

    if (typemask != DB::anyMediaType)
        condList.prepend(typeCondition(QStr("type"), typemask));

    QString cond = condList.join(QStr(" AND "));

    QString query = select;
    if (!cond.isEmpty())
        query += QStr(" WHERE ") + cond;

    query += QStr(" ORDER BY position");

    QList<DB::RawId> positive =
        executeQuery(query.toAscii().constData(), binds).asList<DB::RawId>();

    if (excludeQuery.isEmpty())
        return positive;

    QList<DB::RawId> negative =
        executeQuery(
            (select + QStr(" WHERE ") + excludeQuery.join(QStr(" OR ")))
            .toAscii().constData(),
            excBinds).asList<DB::RawId>();

    return listSubtract(positive, negative);
}

// TODO: remove dependencies to these two
#include "DB/ImageDB.h"
#include "DB/GroupCounter.h"


QMap<QString, uint>
QueryHelper::classify(const QString& category,
                      DB::MediaType typemask,
                      QList<DB::RawId>* scope) const
{
    QMap<QString, uint> result;
    DB::GroupCounter counter( category );

    const QMap<DB::RawId, StringSet > wholeItemMap =
        mediaIdTagsMap(category, typemask);

    QMap<DB::RawId, QStringList> itemMap;

    for (QMap<DB::RawId, StringSet >::const_iterator
             i = wholeItemMap.constBegin(); i != wholeItemMap.constEnd(); ++i) {
        DB::RawId file_id = i.key();
        if (!scope || scope->contains(file_id))
            itemMap[file_id] = i.value().toList();
    }

    // Count images that doesn't contain an item
    if (!scope)
        result[DB::ImageDB::NONE()] = mediaItemCount(typemask) - itemMap.count();
    else
        result[DB::ImageDB::NONE()] = scope->count() - itemMap.count();

    for( QMap<DB::RawId,QStringList>::Iterator mapIt = itemMap.begin(); mapIt != itemMap.end(); ++mapIt ) {
        QStringList list = mapIt.value();
        for( QStringList::ConstIterator listIt = list.constBegin(); listIt != list.constEnd(); ++listIt ) {
            //if ( !alreadyMatched[ *listIt ] ) // We do not want to match "Jesper & Jesper"
                result[ *listIt ]++;
        }
        // TODO: can we do the whole stuff here without list and directly with
        // a set ? So that we don't have to convert it back and forth.
        counter.count( list.toSet() );
    }

    const QMap<QString, uint> groups = counter.result();
    for (QMap<QString, uint>::const_iterator it = groups.constBegin();
         it != groups.constEnd(); ++it) {
        result[it.key()] = it.value();
    }
    return result;
}

DB::StackID QueryHelper::stackFiles(QList<DB::RawId> files)
{
    TransactionGuard transaction(*this);

    const QVariant fileIds(toVariantList(files));
    Cursor fileStackCursor =
        executeQuery(
            "SELECT id, stack_id FROM file WHERE id IN (?)",
            Bindings() << fileIds).cursor();
    QVariant stackId;
    for (fileStackCursor.selectFirstRow();
         fileStackCursor.rowExists();
         fileStackCursor.selectNextRow()) {
        const QVariant curStackId = fileStackCursor.value(1);
        if (!curStackId.isNull()) {
            if (!stackId.isNull() && stackId != curStackId) {
                // Two different stack ids were found
                throw OperationNotPossible(
                    QLatin1String(
                        "Cannot stack files, "
                        "because they are not on a same stack"));
            }
            stackId = curStackId;
        }
    }

    // Now we know, that all of the stack id's that are not null are
    // equal. We'll just go ahead and make an id for the new stack, if
    // there was not one already.
    if (stackId.isNull())
        stackId = executeQuery("SELECT MAX(stack_id)+1 FROM file").firstItem();
    if (stackId.isNull())
        stackId = QVariant(1);

    executeStatement(
        "UPDATE file SET stack_id=? WHERE id IN (?)",
        Bindings() << stackId << fileIds);

    transaction.commit();

    return stackId.toInt();
}

void QueryHelper::unstackFiles(QList<DB::RawId> files)
{
    executeStatement(
        "UPDATE file SET stack_id=NULL WHERE id IN (?)",
        Bindings() << QVariant(toVariantList(files)));
}

QList<DB::RawId> QueryHelper::getStackOfFile(DB::RawId referenceFile) const
{
    // Note: This query works even if stack id of the reference file
    // is NULL, because in SQL "NULL = NULL" evaluates as false and
    // therefore the query returns empty set, if the stack id is NULL.
    // (Tested with SQLite 3.5.9 and MySQL 5.0.60.)
    return
        executeQuery(
            "SELECT id FROM file WHERE stack_id=("
            " SELECT stack_id"
            " FROM file"
            " WHERE id=?)",
            Bindings() << referenceFile).asList<DB::RawId>();
}

QMap<DB::RawId, DB::ImageInfoPtr> QueryHelper::getInfosOfFiles(const QList<DB::RawId>& idList) const
{
    // TODO: remove debug
    qDebug() << "Reading info of files with  ids:" << idList;

    if (idList.isEmpty())
        return QMap<DB::RawId, DB::ImageInfoPtr>();

    //qSort(idList.constBegin(), idList.constEnd());

    QVariant idListVariant(toVariantList(idList));

    Cursor tagDataCursor =
            executeQuery(
                "SELECT file_tag.file_id, category.name, tag.name"
                " FROM file_tag, category, tag"
                " WHERE file_tag.tag_id=tag.id"
                " AND category.id=tag.category_id"
                " AND file_tag.file_id IN (?)",
                Bindings() << idListVariant).cursor();

    typedef QPair<QString, QString> Tag;
    QMap<DB::RawId, QSet<Tag> > tagMap;

    for (tagDataCursor.selectFirstRow();
         tagDataCursor.rowExists();
         tagDataCursor.selectNextRow()) {
        const RowData tagRow(tagDataCursor.getCurrentRow());
        tagMap[DB::RawId(tagRow[0].toInt())].insert(qMakePair(tagRow[1].toString(), tagRow[2].toString()));
    }

    Cursor mainDataCursor =
        executeQuery(
            "SELECT directory.path, f.filename, f.md5sum, f.type,"
            " f.label, f.description,"
            " f.time_start, f.time_end, f.width, f.height, f.angle,"
            " f.rating, f.stack_id, f.stack_position,"
            " f.gps_longitude, f.gps_latitude, f.gps_altitude, f.gps_precision, f.id"
            " FROM file AS f, directory"
            " WHERE f.directory_id = directory.id"
            " AND f.id IN (?)",
            Bindings() << idListVariant).cursor();

    QMap<DB::RawId, DB::ImageInfoPtr> infos;

    for (mainDataCursor.selectFirstRow();
         mainDataCursor.rowExists();
         mainDataCursor.selectNextRow()) {
        const RowData row(mainDataCursor.getCurrentRow());
        Q_ASSERT(row.count() == 19);

        const DB::RawId fileId(row[18].toInt());

        // TODO: remove ugly const cast
        std::auto_ptr<SQLImageInfo> info(new SQLImageInfo(const_cast<QueryHelper*>(this), fileId));

        info->delaySavingChanges(true);

        info->setFileName(makeFullName(row[0].toString(), row[1].toString()));
        info->setMD5Sum(DB::MD5(row[2].toString()));
        info->setMediaType(static_cast<DB::MediaType>(row[3].toInt()));
        info->setLabel(row[4].toString());
        info->setDescription(row[5].toString());
        QDateTime startDate = row[6].toDateTime();
        QDateTime endDate = row[7].toDateTime();
        info->setDate(DB::ImageDate(startDate, endDate));
        int width = row[8].isNull() ? -1 : row[8].toInt();
        int height = row[9].isNull() ? -1 :row[9].toInt();
        info->setSize(QSize(width, height));
        info->setAngle(row[10].toInt());
        info->setRating(row[11].isNull() ? -1 : row[11].toInt());
        info->setStackId(row[12].isNull() ? 0 : row[12].toInt());
        info->setStackOrder(row[13].isNull() ? 0 : row[13].toInt());
        if (!row[14].isNull() && !row[15].isNull() && !row[16].isNull()) {
            int precision = row[17].isNull() ? DB::GpsCoordinates::NoPrecisionData : row[17].toInt();
            info->setGeoPosition(
                DB::GpsCoordinates(
                    row[14].toDouble(),
                    row[15].toDouble(),
                    row[16].toDouble(),
                    precision));
        }
        else
            info->setGeoPosition(DB::GpsCoordinates());

        Q_FOREACH(const Tag& tag, tagMap[fileId])
            info->addCategoryInfo(tag.first, tag.second);

        info->markAsNotNullAndNotDirty();
        info->delaySavingChanges(false);  // Doesn't save because isDirty() == false

        infos[fileId] = DB::ImageInfoPtr(info.release());
    }

    // Note: There might be cases when some (or even any) of the
    // requested ids are not in the file table and that is not
    // necessarily an error. So we don't do any checks for that.
    return infos;
}
