// SPDX-FileCopyrightText: 2006-2008 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2006-2014 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2007 Laurent Montel <montel@kde.org>
// SPDX-FileCopyrightText: 2008-2011 Jan Kundrát <jkt@flaska.net>
// SPDX-FileCopyrightText: 2008-2009 Henner Zeller <h.zeller@acm.org>
// SPDX-FileCopyrightText: 2012 Yuri Chornoivan <yurchor@ukr.net>
// SPDX-FileCopyrightText: 2012-2013 Miika Turkia <miika.turkia@gmail.com>
// SPDX-FileCopyrightText: 2018-2020 Robert Krawitz <rlk@alum.mit.edu>
// SPDX-FileCopyrightText: 2012-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2014-2024 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "FileWriter.h"

#include "CompressFileInfo.h"
#include "ElementWriter.h"
#include "NumberedBackup.h"

#include <DB/Category.h>
#include <DB/ImageDB.h>
#include <DB/TagInfo.h>
#include <Utilities/List.h>
#include <kpabase/Logging.h>
#include <kpabase/SettingsData.h>
#include <kpabase/UIDelegate.h>

#include <KLocalizedString>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QMutexLocker>
#include <QXmlStreamWriter>
#include <QRegularExpression>

#include <utility>

//
//
//
//  +++++++++++++++++++++++++++++++ REMEMBER ++++++++++++++++++++++++++++++++
//
//
//
//
// Update DB::ImageDB::fileVersion every time you update the file format!
//
//
//
//
//
//
//
//
// (sorry for the noise, but it is really important :-)

using Utilities::StringSet;

namespace
{
constexpr QFileDevice::Permissions FILE_PERMISSIONS { QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::WriteGroup | QFile::ReadOther };
}

void DB::FileWriter::save(const QString &fileName, bool isAutoSave)
{
    setUseCompressedFileFormat(Settings::SettingsData::instance()->useCompressedIndexXML());

    if (!isAutoSave)
        NumberedBackup(m_db->uiDelegate(), fileName).makeNumberedBackup();

    // prepare XML document for saving:
    m_db->m_categoryCollection.initIdMap();
    QFile out(fileName + QStringLiteral(".tmp"));
    if (!out.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_db->uiDelegate().error(
            DB::LogMessage { DBLog(), QStringLiteral("Error saving to file '%1': %2").arg(out.fileName(), out.errorString()) }, i18n("<p>Could not save the image database to XML.</p>"
                                                                                                                                     "File %1 could not be opened because of the following error: %2",
                                                                                                                                     out.fileName(), out.errorString()),
            i18n("Error while saving..."));
        return;
    }
    if (!out.setPermissions(FILE_PERMISSIONS)) {
        qCWarning(DBLog, "Could not set permissions on file %s!", qPrintable(out.fileName()));
    }
    QElapsedTimer timer;
    if (TimingLog().isDebugEnabled())
        timer.start();
    QXmlStreamWriter writer(&out);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();

    {
        ElementWriter dummy(writer, QStringLiteral("KPhotoAlbum"));
        writer.writeAttribute(QStringLiteral("version"), QString::number(DB::ImageDB::fileVersion()));
        writer.writeAttribute(QStringLiteral("compressed"), QString::number(useCompressedFileFormat()));

        saveCategories(writer);
        saveImages(writer);
        saveBlockList(writer);
        saveMemberGroups(writer);
        // saveSettings(writer);
        saveGlobalSortOrder(writer);
    }
    writer.writeEndDocument();
    qCDebug(TimingLog) << "DB::FileWriter::save(): Saving took" << timer.elapsed() << "ms";

    // State: XML file has previous DB version, temp file has the current version.

    // original file can be safely deleted
    if ((!QFile::remove(fileName)) && QFile::exists(fileName)) {
        m_db->uiDelegate().error(
            DB::LogMessage { DBLog(), QStringLiteral("Removal of file '%1' failed.").arg(fileName) }, i18n("<p>Failed to remove old version of image database.</p>"
                                                                                                           "<p>Please try again or replace the file %1 with file %2 manually!</p>",
                                                                                                           fileName, out.fileName()),
            i18n("Error while saving..."));
        return;
    }
    // State: XML file doesn't exist, temp file has the current version.
    if (!out.rename(fileName)) {
        m_db->uiDelegate().error(
            DB::LogMessage { DBLog(), QStringLiteral("Renaming '%1' to '%2' failed.").arg(out.fileName().arg(fileName)) }, i18n("<p>Failed to move temporary XML file to permanent location.</p>"
                                                                                                                                "<p>Please try again or rename file %1 to %2 manually!</p>",
                                                                                                                                out.fileName(), fileName),
            i18n("Error while saving..."));
        // State: temp file has the current version.
        return;
    }
    // State: XML file has the current version.
}

void DB::FileWriter::saveCategories(QXmlStreamWriter &writer)
{
    const QStringList categories = DB::ImageDB::instance()->categoryCollection()->categoryNames();
    ElementWriter dummy(writer, QStringLiteral("Categories"));

    DB::CategoryPtr tokensCategory = DB::ImageDB::instance()->categoryCollection()->categoryForSpecial(DB::Category::TokensCategory);
    const DB::TagInfo *untaggedTag = DB::ImageDB::instance()->untaggedTag();
    for (const QString &name : categories) {
        DB::CategoryPtr category = DB::ImageDB::instance()->categoryCollection()->categoryForName(name);

        if (!shouldSaveCategory(name)) {
            continue;
        }

        ElementWriter dummy(writer, QStringLiteral("Category"));
        writer.writeAttribute(QStringLiteral("name"), name);
        writer.writeAttribute(QStringLiteral("icon"), category->iconName());
        writer.writeAttribute(QStringLiteral("show"), QString::number(category->doShow()));
        writer.writeAttribute(QStringLiteral("viewtype"), QString::number(category->viewType()));
        writer.writeAttribute(QStringLiteral("thumbnailsize"), QString::number(category->thumbnailSize()));
        writer.writeAttribute(QStringLiteral("positionable"), QString::number(category->positionable()));
        if (category == tokensCategory) {
            writer.writeAttribute(QStringLiteral("meta"), QStringLiteral("tokens"));
        }

        // As bug 423334 shows, it is easy to forget to add a group to the respective category
        // when it's created. We can not enforce correct creation of member groups in our API,
        // but we can prevent incorrect data from entering the XML file.
        const auto categoryItems = Utilities::mergeListsUniqly(category->items(), m_db->memberMap().groups(name));
        for (const QString &tagName : categoryItems) {
            ElementWriter dummy(writer, QStringLiteral("value"));
            writer.writeAttribute(QStringLiteral("value"), tagName);
            writer.writeAttribute(QStringLiteral("id"), QString::number(category->idForName(tagName)));
            QDate birthDate = category->birthDate(tagName);
            if (!birthDate.isNull())
                writer.writeAttribute(QStringLiteral("birthDate"), birthDate.toString(Qt::ISODate));
            if (untaggedTag && untaggedTag->category() == category.data() && untaggedTag->tagName() == tagName) {
                writer.writeAttribute(QStringLiteral("meta"), QStringLiteral("mark-untagged"));
            }
        }
    }
}

void DB::FileWriter::saveImages(QXmlStreamWriter &writer)
{
    DB::ImageInfoList list = m_db->m_images;

    // Copy files from clipboard to end of overview, so we don't loose them
    const auto clipBoardImages = m_db->m_clipboard;
    for (const DB::ImageInfoPtr &infoPtr : clipBoardImages) {
        list.append(infoPtr);
    }

    {
        ElementWriter dummy(writer, QStringLiteral("images"));

        for (const DB::ImageInfoPtr &infoPtr : std::as_const(list)) {
            save(writer, infoPtr);
        }
    }
}

void DB::FileWriter::saveBlockList(QXmlStreamWriter &writer)
{
    ElementWriter dummy(writer, QStringLiteral("blocklist"));
    QList<DB::FileName> blockList(m_db->m_blockList.begin(), m_db->m_blockList.end());
    // sort blocklist to get diffable files
    std::sort(blockList.begin(), blockList.end());
    for (const DB::FileName &block : std::as_const(blockList)) {
        ElementWriter dummy(writer, QStringLiteral("block"));
        writer.writeAttribute(QStringLiteral("file"), block.relative());
    }
}

void DB::FileWriter::saveMemberGroups(QXmlStreamWriter &writer)
{
    if (m_db->m_members.isEmpty())
        return;

    ElementWriter dummy(writer, QStringLiteral("member-groups"));
    for (QMap<QString, QMap<QString, StringSet>>::ConstIterator memberMapIt = m_db->m_members.memberMap().constBegin();
         memberMapIt != m_db->m_members.memberMap().constEnd(); ++memberMapIt) {
        const QString categoryName = memberMapIt.key();

        // FIXME (l3u): This can happen when an empty sub-category (group) is present.
        //              Would be fine to fix the reason why this happens in the first place.
        if (categoryName.isEmpty()) {
            continue;
        }

        if (!shouldSaveCategory(categoryName))
            continue;

        QMap<QString, StringSet> groupMap = memberMapIt.value();
        for (QMap<QString, StringSet>::ConstIterator groupMapIt = groupMap.constBegin(); groupMapIt != groupMap.constEnd(); ++groupMapIt) {

            // FIXME (l3u): This can happen when an empty sub-category (group) is present.
            //              Would be fine to fix the reason why this happens in the first place.
            if (groupMapIt.key().isEmpty()) {
                continue;
            }

            if (useCompressedFileFormat()) {
                const StringSet members = groupMapIt.value();
                ElementWriter dummy(writer, QStringLiteral("member"));
                writer.writeAttribute(QStringLiteral("category"), categoryName);
                writer.writeAttribute(QStringLiteral("group-name"), groupMapIt.key());
                QStringList idList;
                for (const QString &member : members) {
                    DB::CategoryPtr catPtr = m_db->m_categoryCollection.categoryForName(categoryName);
                    if (catPtr->idForName(member) == 0)
                        qCWarning(DBLog) << "Member" << member << "in group" << categoryName << "->" << groupMapIt.key() << "has no id!";
                    idList.append(QString::number(catPtr->idForName(member)));
                }
                std::sort(idList.begin(), idList.end());
                writer.writeAttribute(QStringLiteral("members"), idList.join(QStringLiteral(",")));
            } else {
                const auto groupMapItValue = groupMapIt.value();
                QStringList members(groupMapItValue.begin(), groupMapItValue.end());
                std::sort(members.begin(), members.end());
                for (const QString &member : std::as_const(members)) {
                    ElementWriter dummy(writer, QStringLiteral("member"));
                    writer.writeAttribute(QStringLiteral("category"), memberMapIt.key());
                    writer.writeAttribute(QStringLiteral("group-name"), groupMapIt.key());
                    writer.writeAttribute(QStringLiteral("member"), member);
                }

                // Add an entry even if the group is empty
                // (this is not necessary for the compressed format)
                if (members.size() == 0) {
                    ElementWriter dummy(writer, QStringLiteral("member"));
                    writer.writeAttribute(QStringLiteral("category"), memberMapIt.key());
                    writer.writeAttribute(QStringLiteral("group-name"), groupMapIt.key());
                }
            }
        }
    }
}

void DB::FileWriter::saveGlobalSortOrder(QXmlStreamWriter &writer)
{
    ElementWriter dummy(writer, QStringLiteral("global-sort-order"));
    for (const auto &item : m_db->categoryCollection()->globalSortOrder()->modifiedSortOrder()) {
        ElementWriter dummy(writer, QStringLiteral("item"));
        writer.writeAttribute(QStringLiteral("category"), item.category);
        writer.writeAttribute(QStringLiteral("item"), item.item);
    }
}

/*
Perhaps, we may need this later ;-)

void DB::FileWriter::saveSettings(QXmlStreamWriter& writer)
{
    ElementWriter dummy(writer, settingsString);

    QMap<QString, QString> settings;
    // For testing
    settings.insert(QStringLiteral("tokensCategory"), QStringLiteral("Tokens"));
    settings.insert(QStringLiteral("untaggedCategory"), QStringLiteral("Events"));
    settings.insert(QStringLiteral("untaggedTag"), QStringLiteral("untagged"));

    QMapIterator<QString, QString> settingsIterator(settings);
    while (settingsIterator.hasNext()) {
        ElementWriter dummy(writer, settingString);
        settingsIterator.next();
        writer.writeAttribute(QStringLiteral("key"), escape(settingsIterator.key()));
        writer.writeAttribute(QStringLiteral("value"), escape(settingsIterator.value()));
    }
}
*/

static const QString &stdDateTimeToString(const Utilities::FastDateTime &date)
{
    static QString s_lastDateTimeString;
    static Utilities::FastDateTime s_lastDateTime;
    static QMutex s_lastDateTimeLocker;
    QMutexLocker dummy(&s_lastDateTimeLocker);
    if (date.isValid() && date != s_lastDateTime) {
        s_lastDateTime = date;
        s_lastDateTimeString = date.toString(Qt::ISODate);
    }
    return s_lastDateTimeString;
}

void DB::FileWriter::save(QXmlStreamWriter &writer, const DB::ImageInfoPtr &info)
{
    ElementWriter dummy(writer, QStringLiteral("image"));
    writer.writeAttribute(QStringLiteral("file"), info->fileName().relative());
    if (info->label() != QFileInfo(info->fileName().relative()).completeBaseName())
        writer.writeAttribute(QStringLiteral("label"), info->label());
    if (!info->description().isEmpty())
        writer.writeAttribute(QStringLiteral("description"), info->description());

    DB::ImageDate date = info->date();
    Utilities::FastDateTime start = date.start();
    Utilities::FastDateTime end = date.end();

    writer.writeAttribute(QStringLiteral("startDate"), stdDateTimeToString(start));
    if (start != end)
        writer.writeAttribute(QStringLiteral("endDate"), stdDateTimeToString(end));

    if (info->angle() != 0)
        writer.writeAttribute(QStringLiteral("angle"), QString::number(info->angle()));
    writer.writeAttribute(QStringLiteral("md5sum"), info->MD5Sum().toHexString());
    writer.writeAttribute(QStringLiteral("width"), QString::number(info->size().width()));
    writer.writeAttribute(QStringLiteral("height"), QString::number(info->size().height()));

    if (info->rating() != -1) {
        writer.writeAttribute(QStringLiteral("rating"), QString::number(info->rating()));
    }

    if (info->stackId()) {
        writer.writeAttribute(QStringLiteral("stackId"), QString::number(info->stackId()));
        writer.writeAttribute(QStringLiteral("stackOrder"), QString::number(info->stackOrder()));
    }

    if (info->isVideo())
        writer.writeAttribute(QStringLiteral("videoLength"), QString::number(info->videoLength()));

    if (useCompressedFileFormat())
        writeCategoriesCompressed(writer, info);
    else
        writeCategories(writer, info);
}

QString DB::FileWriter::areaToString(QRect area) const
{
    QStringList areaString;
    areaString.append(QString::number(area.x()));
    areaString.append(QString::number(area.y()));
    areaString.append(QString::number(area.width()));
    areaString.append(QString::number(area.height()));
    return areaString.join(QStringLiteral(" "));
}

void DB::FileWriter::writeCategories(QXmlStreamWriter &writer, const DB::ImageInfoPtr &info)
{
    ElementWriter topElm(writer, QStringLiteral("options"), false);

    QStringList grps = info->availableCategories();
    // in contrast to CategoryCollection::categories, availableCategories is randomly sorted (since it is now a QHash)
    grps.sort();
    for (const QString &name : grps) {
        if (!shouldSaveCategory(name))
            continue;

        ElementWriter categoryElm(writer, QStringLiteral("option"), false);

        const auto itemsOfCategory = info->itemsOfCategory(name);
        QStringList items(itemsOfCategory.begin(), itemsOfCategory.end());
        std::sort(items.begin(), items.end());
        if (!items.isEmpty()) {
            topElm.writeStartElement();
            categoryElm.writeStartElement();
            writer.writeAttribute(QStringLiteral("name"), name);
        }

        for (const QString &itemValue : std::as_const(items)) {
            ElementWriter dummy(writer, QStringLiteral("value"));
            writer.writeAttribute(QStringLiteral("value"), itemValue);

            QRect area = info->areaForTag(name, itemValue);
            if (!area.isNull()) {
                writer.writeAttribute(QStringLiteral("area"), areaToString(area));
            }
        }
    }
}

void DB::FileWriter::writeCategoriesCompressed(QXmlStreamWriter &writer, const DB::ImageInfoPtr &info)
{
    QMap<QString, QList<QPair<QString, QRect>>> positionedTags;

    const QList<DB::CategoryPtr> categoryList = DB::ImageDB::instance()->categoryCollection()->categories();
    for (const DB::CategoryPtr &category : categoryList) {
        QString categoryName = category->name();

        if (!shouldSaveCategory(categoryName))
            continue;

        const StringSet items = info->itemsOfCategory(categoryName);
        if (!items.empty()) {
            QStringList idList;

            for (const QString &itemValue : items) {
                QRect area = info->areaForTag(categoryName, itemValue);

                if (area.isValid()) {
                    // Positioned tags can't be stored in the "fast" format
                    // so we have to handle them separately
                    positionedTags[categoryName] << QPair<QString, QRect>(itemValue, area);
                } else {
                    int id = category->idForName(itemValue);
                    idList.append(QString::number(id));
                }
            }

            // Possibly all ids of a category have area information, so only
            // write the category attribute if there are actually ids to write
            if (!idList.isEmpty()) {
                std::sort(idList.begin(), idList.end());
                writer.writeAttribute(escape(categoryName, DB::ImageDB::fileVersion()),
                                      idList.join(QStringLiteral(",")));
            }
        }
    }

    // Add a "readable" sub-element for the positioned tags
    // FIXME: can this be merged with the code in writeCategories()?
    if (!positionedTags.isEmpty()) {
        ElementWriter topElm(writer, QStringLiteral("options"), false);
        topElm.writeStartElement();

        QMapIterator<QString, QList<QPair<QString, QRect>>> categoryWithAreas(positionedTags);
        while (categoryWithAreas.hasNext()) {
            categoryWithAreas.next();

            ElementWriter categoryElm(writer, QStringLiteral("option"), false);
            categoryElm.writeStartElement();
            writer.writeAttribute(QStringLiteral("name"), categoryWithAreas.key());

            QList<QPair<QString, QRect>> areas = categoryWithAreas.value();
            std::sort(areas.begin(), areas.end(),
                      [](QPair<QString, QRect> a, QPair<QString, QRect> b) { return a.first < b.first; });
            for (const auto &positionedTag : std::as_const(areas)) {
                ElementWriter dummy(writer, QStringLiteral("value"));
                writer.writeAttribute(QStringLiteral("value"), positionedTag.first);
                writer.writeAttribute(QStringLiteral("area"), areaToString(positionedTag.second));
            }
        }
    }
}

bool DB::FileWriter::shouldSaveCategory(const QString &categoryName) const
{
    // Profiling indicated that this function was a hotspot, so this cache improved saving speed with 25%
    static QHash<QString, bool> cache;
    if (cache.contains(categoryName))
        return cache[categoryName];

    // A few bugs has shown up, where an invalid category name has crashed KPA. It therefore checks for such invalid names here.
    if (!m_db->m_categoryCollection.categoryForName(categoryName)) {
        qCWarning(DBLog, "Invalid category name: %s", qPrintable(categoryName));
        cache.insert(categoryName, false);
        return false;
    }

    const auto category = m_db->m_categoryCollection.categoryForName(categoryName).data();
    Q_ASSERT(category);
    const bool shouldSave = category->shouldSave();
    cache.insert(categoryName, shouldSave);
    return shouldSave;
}

/**
 * @brief Escape problematic characters in a string that forms an XML attribute name.
 *
 * N.B.: Attribute values do not need to be escaped!
 * @see DB::FileReader::unescape
 *
 * @param str the string to be escaped
 * @param int the db version we want to escape for
 * @return the escaped string
 */
QString DB::FileWriter::escape(const QString &str, int fileVersion)
{
    static bool s_hashUsesCompressedFormat = useCompressedFileFormat();
    static QHash<QString, QString> s_cache;
    static int s_cacheVersion = -1;

    if (s_hashUsesCompressedFormat != useCompressedFileFormat() || s_cacheVersion != fileVersion) {
        s_cache.clear();
        s_cacheVersion = fileVersion;
        s_hashUsesCompressedFormat = useCompressedFileFormat();
    }

    if (s_cache.contains(str)) {
        return s_cache.value(str);
    }

    // Up to db v10, some Latin-1-only compatible escaping has been done using regular expressions
    // for the "compressed" format. Also, there was some space-underscore substitution for the
    // "readable" format. We need this when reading a pre-v11 db, so we still provide this algorithm
    // here:

    if (fileVersion <= 10) {
        QString escaped;

        if (!useCompressedFileFormat()) {
            escaped = str;
            escaped.replace(QStringLiteral(" "), QStringLiteral("_"));

        } else {
            if (useCompressedFileFormat()) {
                static const QRegularExpression rx(QStringLiteral("([^a-zA-Z0-9:_])"));
                QString tmp(str);
                while (true) {
                    const auto match = rx.match(tmp);
                    if (match.hasMatch()) {
                        escaped += tmp.left(match.capturedStart())
                                   + QString::asprintf("_.%0X", match.captured().data()->toLatin1());
                        tmp = tmp.mid(match.capturedStart() + match.capturedLength(), -1);
                    } else {
                        escaped += tmp;
                        break;
                    }
                }
            }
        }

        s_cache.insert(str, escaped);
        return escaped;
    }

    if (!useCompressedFileFormat()) {
        // If we use the "readable" format, we don't need escaping:
        return str;

    } else {
        // If we use the "compressed" file format, we have to do some escaping, because the category
        // names are used as XML attributes and we can't use all characters for them. Beginning from
        // db v11, we use a modified percent encoding provided by QByteArray that will work for all
        // input strings and covers the whole Unicode range:

        // The first character of an XML attribute must be a NameStartChar. That is a-z,
        // A-Z, ":" or "_". From the second position on, also 0-9, "." and "-" are allowed
        // (cf. https://www.w3.org/TR/xml/ for the full specification).
        //
        // To keep it simple, we escape everything that is not a small or capital letter, and use
        // the underscore as our escaping char. This way, nothing can go wrong, no matter what the
        // given string contains, and at which position.

        // By default, QByteArray::toPercentEncoding encodes everything that is not a-z, A-Z, 0-9,
        // "-", ".", "_" or "~". We thus have to add some more chars to include:
        static const QByteArray s_escapeIncludes = QStringLiteral("-._~0123456789").toUtf8();

        const auto escaped = QString::fromUtf8(
            str.toUtf8().toPercentEncoding(QByteArray(), s_escapeIncludes, '_'));
        s_cache.insert(str, escaped);

        // We always start with a ":", so that we can't collide with our internal attribute names
        return QStringLiteral(":%1").arg(escaped);
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:
