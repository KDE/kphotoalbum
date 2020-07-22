/* Copyright (C) 2003-2020 The KPhotoAlbum Development Team

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "FileWriter.h"

#include "CompressFileInfo.h"
#include "Database.h"
#include "ElementWriter.h"
#include "Logging.h"
#include "NumberedBackup.h"
#include "XMLCategory.h"

#include <DB/UIDelegate.h>
#include <MainWindow/Logging.h>
#include <Settings/SettingsData.h>
#include <Utilities/List.h>

#include <KLocalizedString>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QXmlStreamWriter>

//
//
//
//  +++++++++++++++++++++++++++++++ REMEMBER ++++++++++++++++++++++++++++++++
//
//
//
//
// Update XMLDB::Database::fileVersion every time you update the file format!
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

void XMLDB::FileWriter::save(const QString &fileName, bool isAutoSave)
{
    setUseCompressedFileFormat(Settings::SettingsData::instance()->useCompressedIndexXML());

    if (!isAutoSave)
        NumberedBackup(m_db->uiDelegate()).makeNumberedBackup();

    // prepare XML document for saving:
    m_db->m_categoryCollection.initIdMap();
    QFile out(fileName + QString::fromLatin1(".tmp"));
    if (!out.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_db->uiDelegate().sorry(
            QString::fromUtf8("Error saving to file '%1': %2").arg(out.fileName()).arg(out.errorString()), i18n("<p>Could not save the image database to XML.</p>"
                                                                                                                "File %1 could not be opened because of the following error: %2",
                                                                                                                out.fileName(), out.errorString()),
            i18n("Error while saving..."));
        return;
    }
    QElapsedTimer timer;
    if (TimingLog().isDebugEnabled())
        timer.start();
    QXmlStreamWriter writer(&out);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();

    {
        ElementWriter dummy(writer, QString::fromLatin1("KPhotoAlbum"));
        writer.writeAttribute(QString::fromLatin1("version"), QString::number(Database::fileVersion()));
        writer.writeAttribute(QString::fromLatin1("compressed"), QString::number(useCompressedFileFormat()));

        saveCategories(writer);
        saveImages(writer);
        saveBlockList(writer);
        saveMemberGroups(writer);
        //saveSettings(writer);
    }
    writer.writeEndDocument();
    qCDebug(TimingLog) << "XMLDB::FileWriter::save(): Saving took" << timer.elapsed() << "ms";

    // State: index.xml has previous DB version, index.xml.tmp has the current version.

    // original file can be safely deleted
    if ((!QFile::remove(fileName)) && QFile::exists(fileName)) {
        m_db->uiDelegate().sorry(
            QString::fromUtf8("Removal of file '%1' failed.").arg(fileName), i18n("<p>Failed to remove old version of image database.</p>"
                                                                                  "<p>Please try again or replace the file %1 with file %2 manually!</p>",
                                                                                  fileName, out.fileName()),
            i18n("Error while saving..."));
        return;
    }
    // State: index.xml doesn't exist, index.xml.tmp has the current version.
    if (!out.rename(fileName)) {
        m_db->uiDelegate().sorry(
            QString::fromUtf8("Renaming index.xml to '%1' failed.").arg(out.fileName()), i18n("<p>Failed to move temporary XML file to permanent location.</p>"
                                                                                              "<p>Please try again or rename file %1 to %2 manually!</p>",
                                                                                              out.fileName(), fileName),
            i18n("Error while saving..."));
        // State: index.xml.tmp has the current version.
        return;
    }
    // State: index.xml has the current version.
}

void XMLDB::FileWriter::saveCategories(QXmlStreamWriter &writer)
{
    QStringList categories = DB::ImageDB::instance()->categoryCollection()->categoryNames();
    ElementWriter dummy(writer, QString::fromLatin1("Categories"));

    DB::CategoryPtr tokensCategory = DB::ImageDB::instance()->categoryCollection()->categoryForSpecial(DB::Category::TokensCategory);
    for (QString name : categories) {
        DB::CategoryPtr category = DB::ImageDB::instance()->categoryCollection()->categoryForName(name);

        if (!shouldSaveCategory(name)) {
            continue;
        }

        ElementWriter dummy(writer, QString::fromUtf8("Category"));
        writer.writeAttribute(QString::fromUtf8("name"), name);
        writer.writeAttribute(QString::fromUtf8("icon"), category->iconName());
        writer.writeAttribute(QString::fromUtf8("show"), QString::number(category->doShow()));
        writer.writeAttribute(QString::fromUtf8("viewtype"), QString::number(category->viewType()));
        writer.writeAttribute(QString::fromUtf8("thumbnailsize"), QString::number(category->thumbnailSize()));
        writer.writeAttribute(QString::fromUtf8("positionable"), QString::number(category->positionable()));
        if (category == tokensCategory) {
            writer.writeAttribute(QString::fromUtf8("meta"), QString::fromUtf8("tokens"));
        }

        // As bug 423334 shows, it is easy to forget to add a group to the respective category
        // when it's created. We can not enforce correct creation of member groups in our API,
        // but we can prevent incorrect data from entering index.xml.
        const auto categoryItems = Utilities::mergeListsUniqly(category->items(), m_db->memberMap().groups(name));
        for (const QString &tagName : categoryItems) {
            ElementWriter dummy(writer, QString::fromLatin1("value"));
            writer.writeAttribute(QString::fromLatin1("value"), tagName);
            writer.writeAttribute(QString::fromLatin1("id"),
                                  QString::number(static_cast<XMLCategory *>(category.data())->idForName(tagName)));
            QDate birthDate = category->birthDate(tagName);
            if (!birthDate.isNull())
                writer.writeAttribute(QString::fromUtf8("birthDate"), birthDate.toString(Qt::ISODate));
        }
    }
}

void XMLDB::FileWriter::saveImages(QXmlStreamWriter &writer)
{
    DB::ImageInfoList list = m_db->m_images;

    // Copy files from clipboard to end of overview, so we don't loose them
    const auto clipBoardImages = m_db->m_clipboard;
    for (const DB::ImageInfoPtr &infoPtr : clipBoardImages) {
        list.append(infoPtr);
    }

    {
        ElementWriter dummy(writer, QString::fromLatin1("images"));

        for (const DB::ImageInfoPtr &infoPtr : qAsConst(list)) {
            save(writer, infoPtr);
        }
    }
}

void XMLDB::FileWriter::saveBlockList(QXmlStreamWriter &writer)
{
    ElementWriter dummy(writer, QString::fromLatin1("blocklist"));
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QList<DB::FileName> blockList(m_db->m_blockList.begin(), m_db->m_blockList.end());
#else
    QList<DB::FileName> blockList = m_db->m_blockList.toList();
#endif
    // sort blocklist to get diffable files
    std::sort(blockList.begin(), blockList.end());
    for (const DB::FileName &block : qAsConst(blockList)) {
        ElementWriter dummy(writer, QString::fromLatin1("block"));
        writer.writeAttribute(QString::fromLatin1("file"), block.relative());
    }
}

void XMLDB::FileWriter::saveMemberGroups(QXmlStreamWriter &writer)
{
    if (m_db->m_members.isEmpty())
        return;

    ElementWriter dummy(writer, QString::fromLatin1("member-groups"));
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
                ElementWriter dummy(writer, QString::fromLatin1("member"));
                writer.writeAttribute(QString::fromLatin1("category"), categoryName);
                writer.writeAttribute(QString::fromLatin1("group-name"), groupMapIt.key());
                QStringList idList;
                for (const QString &member : members) {
                    DB::CategoryPtr catPtr = m_db->m_categoryCollection.categoryForName(categoryName);
                    XMLCategory *category = static_cast<XMLCategory *>(catPtr.data());
                    if (category->idForName(member) == 0)
                        qCWarning(XMLDBLog) << "Member" << member << "in group" << categoryName << "->" << groupMapIt.key() << "has no id!";
                    idList.append(QString::number(category->idForName(member)));
                }
                std::sort(idList.begin(), idList.end());
                writer.writeAttribute(QString::fromLatin1("members"), idList.join(QString::fromLatin1(",")));
            } else {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
                const auto groupMapItValue = groupMapIt.value();
                QStringList members(groupMapItValue.begin(), groupMapItValue.end());
#else
                QStringList members = groupMapIt.value().toList();
#endif
                std::sort(members.begin(), members.end());
                for (const QString &member : qAsConst(members)) {
                    ElementWriter dummy(writer, QString::fromLatin1("member"));
                    writer.writeAttribute(QString::fromLatin1("category"), memberMapIt.key());
                    writer.writeAttribute(QString::fromLatin1("group-name"), groupMapIt.key());
                    writer.writeAttribute(QString::fromLatin1("member"), member);
                }

                // Add an entry even if the group is empty
                // (this is not necessary for the compressed format)
                if (members.size() == 0) {
                    ElementWriter dummy(writer, QString::fromLatin1("member"));
                    writer.writeAttribute(QString::fromLatin1("category"), memberMapIt.key());
                    writer.writeAttribute(QString::fromLatin1("group-name"), groupMapIt.key());
                }
            }
        }
    }
}

/*
Perhaps, we may need this later ;-)

void XMLDB::FileWriter::saveSettings(QXmlStreamWriter& writer)
{
    static QString settingsString = QString::fromUtf8("settings");
    static QString settingString = QString::fromUtf8("setting");
    static QString keyString = QString::fromUtf8("key");
    static QString valueString = QString::fromUtf8("value");

    ElementWriter dummy(writer, settingsString);

    QMap<QString, QString> settings;
    // For testing
    settings.insert(QString::fromUtf8("tokensCategory"), QString::fromUtf8("Tokens"));
    settings.insert(QString::fromUtf8("untaggedCategory"), QString::fromUtf8("Events"));
    settings.insert(QString::fromUtf8("untaggedTag"), QString::fromUtf8("untagged"));

    QMapIterator<QString, QString> settingsIterator(settings);
    while (settingsIterator.hasNext()) {
        ElementWriter dummy(writer, settingString);
        settingsIterator.next();
        writer.writeAttribute(keyString, escape(settingsIterator.key()));
        writer.writeAttribute(valueString, escape(settingsIterator.value()));
    }
}
*/

void XMLDB::FileWriter::save(QXmlStreamWriter &writer, const DB::ImageInfoPtr &info)
{
    ElementWriter dummy(writer, QString::fromLatin1("image"));
    writer.writeAttribute(QString::fromLatin1("file"), info->fileName().relative());
    if (info->label() != QFileInfo(info->fileName().relative()).completeBaseName())
        writer.writeAttribute(QString::fromLatin1("label"), info->label());
    if (!info->description().isEmpty())
        writer.writeAttribute(QString::fromLatin1("description"), info->description());

    DB::ImageDate date = info->date();
    QDateTime start = date.start();
    QDateTime end = date.end();

    writer.writeAttribute(QString::fromLatin1("startDate"), start.toString(Qt::ISODate));
    if (start != end)
        writer.writeAttribute(QString::fromLatin1("endDate"), end.toString(Qt::ISODate));

    if (info->angle() != 0)
        writer.writeAttribute(QString::fromLatin1("angle"), QString::number(info->angle()));
    writer.writeAttribute(QString::fromLatin1("md5sum"), info->MD5Sum().toHexString());
    writer.writeAttribute(QString::fromLatin1("width"), QString::number(info->size().width()));
    writer.writeAttribute(QString::fromLatin1("height"), QString::number(info->size().height()));

    if (info->rating() != -1) {
        writer.writeAttribute(QString::fromLatin1("rating"), QString::number(info->rating()));
    }

    if (info->stackId()) {
        writer.writeAttribute(QString::fromLatin1("stackId"), QString::number(info->stackId()));
        writer.writeAttribute(QString::fromLatin1("stackOrder"), QString::number(info->stackOrder()));
    }

    if (info->isVideo())
        writer.writeAttribute(QLatin1String("videoLength"), QString::number(info->videoLength()));

    if (useCompressedFileFormat())
        writeCategoriesCompressed(writer, info);
    else
        writeCategories(writer, info);
}

QString XMLDB::FileWriter::areaToString(QRect area) const
{
    QStringList areaString;
    areaString.append(QString::number(area.x()));
    areaString.append(QString::number(area.y()));
    areaString.append(QString::number(area.width()));
    areaString.append(QString::number(area.height()));
    return areaString.join(QString::fromLatin1(" "));
}

void XMLDB::FileWriter::writeCategories(QXmlStreamWriter &writer, const DB::ImageInfoPtr &info)
{
    ElementWriter topElm(writer, QString::fromLatin1("options"), false);

    QStringList grps = info->availableCategories();
    // in contrast to CategoryCollection::categories, availableCategories is randomly sorted (since it is now a QHash)
    grps.sort();
    for (const QString &name : grps) {
        if (!shouldSaveCategory(name))
            continue;

        ElementWriter categoryElm(writer, QString::fromLatin1("option"), false);

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        const auto itemsOfCategory = info->itemsOfCategory(name);
        QStringList items(itemsOfCategory.begin(), itemsOfCategory.end());
#else
        QStringList items = info->itemsOfCategory(name).toList();
#endif
        std::sort(items.begin(), items.end());
        if (!items.isEmpty()) {
            topElm.writeStartElement();
            categoryElm.writeStartElement();
            writer.writeAttribute(QString::fromLatin1("name"), name);
        }

        for (const QString &itemValue : qAsConst(items)) {
            ElementWriter dummy(writer, QString::fromLatin1("value"));
            writer.writeAttribute(QString::fromLatin1("value"), itemValue);

            QRect area = info->areaForTag(name, itemValue);
            if (!area.isNull()) {
                writer.writeAttribute(QString::fromLatin1("area"), areaToString(area));
            }
        }
    }
}

void XMLDB::FileWriter::writeCategoriesCompressed(QXmlStreamWriter &writer, const DB::ImageInfoPtr &info)
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
                    int id = static_cast<const XMLCategory *>(category.data())->idForName(itemValue);
                    idList.append(QString::number(id));
                }
            }

            // Possibly all ids of a category have area information, so only
            // write the category attribute if there are actually ids to write
            if (!idList.isEmpty()) {
                std::sort(idList.begin(), idList.end());
                writer.writeAttribute(escape(categoryName), idList.join(QString::fromLatin1(",")));
            }
        }
    }

    // Add a "readable" sub-element for the positioned tags
    // FIXME: can this be merged with the code in writeCategories()?
    if (!positionedTags.isEmpty()) {
        ElementWriter topElm(writer, QString::fromLatin1("options"), false);
        topElm.writeStartElement();

        QMapIterator<QString, QList<QPair<QString, QRect>>> categoryWithAreas(positionedTags);
        while (categoryWithAreas.hasNext()) {
            categoryWithAreas.next();

            ElementWriter categoryElm(writer, QString::fromLatin1("option"), false);
            categoryElm.writeStartElement();
            writer.writeAttribute(QString::fromLatin1("name"), categoryWithAreas.key());

            QList<QPair<QString, QRect>> areas = categoryWithAreas.value();
            std::sort(areas.begin(), areas.end(),
                      [](QPair<QString, QRect> a, QPair<QString, QRect> b) { return a.first < b.first; });
            for (const auto &positionedTag : qAsConst(areas)) {
                ElementWriter dummy(writer, QString::fromLatin1("value"));
                writer.writeAttribute(QString::fromLatin1("value"), positionedTag.first);
                writer.writeAttribute(QString::fromLatin1("area"), areaToString(positionedTag.second));
            }
        }
    }
}

bool XMLDB::FileWriter::shouldSaveCategory(const QString &categoryName) const
{
    // Profiling indicated that this function was a hotspot, so this cache improved saving speed with 25%
    static QHash<QString, bool> cache;
    if (cache.contains(categoryName))
        return cache[categoryName];

    // A few bugs has shown up, where an invalid category name has crashed KPA. It therefore checks for such invalid names here.
    if (!m_db->m_categoryCollection.categoryForName(categoryName)) {
        qCWarning(XMLDBLog, "Invalid category name: %s", qPrintable(categoryName));
        cache.insert(categoryName, false);
        return false;
    }

    const bool shouldSave = dynamic_cast<XMLCategory *>(m_db->m_categoryCollection.categoryForName(categoryName).data())->shouldSave();
    cache.insert(categoryName, shouldSave);
    return shouldSave;
}

/**
 * @brief Escape problematic characters in a string that forms an XML attribute name.
 *
 * N.B.: Attribute values do not need to be escaped!
 * @see XMLDB::FileReader::unescape
 *
 * @param str the string to be escaped
 * @return the escaped string
 */
QString XMLDB::FileWriter::escape(const QString &str)
{
    static bool hashUsesCompressedFormat = useCompressedFileFormat();
    static QHash<QString, QString> s_cache;
    if (hashUsesCompressedFormat != useCompressedFileFormat())
        s_cache.clear();

    if (s_cache.contains(str))
        return s_cache[str];

    QString tmp(str);
    // Regex to match characters that are not allowed to start XML attribute names
    const QRegExp rx(QString::fromLatin1("([^a-zA-Z0-9:_])"));
    int pos = 0;

    // Encoding special characters if compressed XML is selected
    if (useCompressedFileFormat()) {
        while ((pos = rx.indexIn(tmp, pos)) != -1) {
            QString before = rx.cap(1);
            QString after = QString::fromLatin1("_.");
            after += QString::fromLocal8Bit(rx.cap(1).toLatin1().toHex());
            tmp.replace(pos, before.length(), after);
            pos += after.length();
        }
    } else
        tmp.replace(QString::fromLatin1(" "), QString::fromLatin1("_"));
    s_cache.insert(str, tmp);
    return tmp;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
