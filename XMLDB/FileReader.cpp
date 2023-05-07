// SPDX-FileCopyrightText: 2006-2007 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2006-2014 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2007 Laurent Montel <montel@kde.org>
// SPDX-FileCopyrightText: 2007-2009 Jan Kundrát <jkt@flaska.net>
// SPDX-FileCopyrightText: 2009 Andrew Coles <andrew.i.coles@googlemail.com>
// SPDX-FileCopyrightText: 2009 Hassan Ibraheem <hasan.ibraheem@gmail.com>
// SPDX-FileCopyrightText: 2009 Henner Zeller <h.zeller@acm.org>
// SPDX-FileCopyrightText: 2012-2020 Yuri Chornoivan <yurchor@ukr.net>
// SPDX-FileCopyrightText: 2012-2013 Miika Turkia <miika.turkia@gmail.com>
// SPDX-FileCopyrightText: 2014-2020 Robert Krawitz <rlk@alum.mit.edu>
// SPDX-FileCopyrightText: 2014-2020 Tobias Leupold <tl@stonemx.de>
// SPDX-FileCopyrightText: 2015 Andreas Neustifter <andreas.neustifter@gmail.com>
// SPDX-FileCopyrightText: 2018 Antoni Bella Pérez <antonibella5@yahoo.com>
// SPDX-FileCopyrightText: 2013-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

// Local includes
#include "FileReader.h"

#include "CompressFileInfo.h"

#include <DB/Category.h>
#include <DB/ImageDB.h>
#include <DB/MD5Map.h>
#include <kpabase/Logging.h>
#include <kpabase/UIDelegate.h>

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QFile>
#include <QHash>
#include <QLocale>
#include <QRegExp>
#include <QStandardPaths>
#include <QTextCodec>
#include <QTextStream>

void XMLDB::FileReader::read(const QString &configFile)
{
    static QString versionString = QString::fromUtf8("version");
    static QString compressedString = QString::fromUtf8("compressed");

    ReaderPtr reader = readConfigFile(configFile);

    ElementInfo info = reader->readNextStartOrStopElement(QString::fromUtf8("KPhotoAlbum"));
    if (!info.isStartToken)
        reader->complainStartElementExpected(QString::fromUtf8("KPhotoAlbum"));

    m_fileVersion = reader->attribute(versionString, QString::fromLatin1("1")).toInt();

    if (m_fileVersion > DB::ImageDB::fileVersion()) {
        DB::UserFeedback ret = m_db->uiDelegate().warningContinueCancel(
            DB::LogMessage { DBLog(), QString::fromLatin1("index.xml version %1 is newer than %2!").arg(m_fileVersion).arg(DB::ImageDB::fileVersion()) },
            i18n("<p>The database file (index.xml) is from a newer version of KPhotoAlbum!</p>"
                 "<p>Chances are you will be able to read this file, but when writing it back, "
                 "information saved in the newer version will be lost</p>"),
            i18n("index.xml version mismatch"), QString::fromLatin1("checkDatabaseFileVersion"));
        if (ret != DB::UserFeedback::Confirm)
            exit(-1);
    }

    setUseCompressedFileFormat(reader->attribute(compressedString).toInt());

    m_db->m_members.setLoading(true);

    loadCategories(reader);
    loadImages(reader);
    loadBlockList(reader);
    loadMemberGroups(reader);
    // loadSettings(reader);

    repairDB();

    m_db->m_members.setLoading(false);

    checkIfImagesAreSorted();
    checkIfAllImagesHaveSizeAttributes();
}

void XMLDB::FileReader::createSpecialCategories()
{
    // Setup the "Folder" category
    m_folderCategory = new DB::Category(i18n("Folder"), QString::fromLatin1("folder"),
                                        DB::Category::TreeView, 32, false);
    m_folderCategory->setType(DB::Category::FolderCategory);
    // The folder category is not stored in the index.xml file,
    // but older versions of KPhotoAlbum stored a stub entry, which we need to remove first:
    if (m_db->m_categoryCollection.categoryForName(m_folderCategory->name()))
        m_db->m_categoryCollection.removeCategory(m_folderCategory->name());
    m_db->m_categoryCollection.addCategory(m_folderCategory);
    m_folderCategory->setShouldSave(false);

    // Setup the "Tokens" category

    DB::CategoryPtr tokenCat;

    if (m_fileVersion >= 7) {
        tokenCat = m_db->m_categoryCollection.categoryForSpecial(DB::Category::TokensCategory);
    } else {
        // Before version 7, the "Tokens" category name wasn't stored to the settings. So ...
        // look for a literal "Tokens" category ...
        tokenCat = m_db->m_categoryCollection.categoryForName(QString::fromUtf8("Tokens"));
        if (!tokenCat) {
            // ... and a translated "Tokens" category if we don't have the literal one.
            tokenCat = m_db->m_categoryCollection.categoryForName(i18n("Tokens"));
        }
        if (tokenCat) {
            // in this case we need to give the tokens category its special meaning:
            m_db->m_categoryCollection.removeCategory(tokenCat->name());
            tokenCat->setType(DB::Category::TokensCategory);
            m_db->m_categoryCollection.addCategory(tokenCat);
        }
    }

    if (!tokenCat) {
        // Create a new "Tokens" category
        tokenCat = new DB::Category(i18n("Tokens"), QString::fromUtf8("tag"),
                                    DB::Category::TreeView, 32, true);
        tokenCat->setType(DB::Category::TokensCategory);
        m_db->m_categoryCollection.addCategory(tokenCat);
    }

    // KPhotoAlbum 2.2 did not write the tokens to the category section,
    // so unless we do this small trick they will not show up when importing.
    for (char ch = 'A'; ch <= 'Z'; ++ch) {
        tokenCat->addItem(QString::fromUtf8("%1").arg(QChar::fromLatin1(ch)));
    }

    // Setup the "Media Type" category
    DB::CategoryPtr mediaCat;
    mediaCat = new DB::Category(i18n("Media Type"), QString::fromLatin1("view-categories"),
                                DB::Category::TreeView, 32, false);
    mediaCat->addItem(i18n("Image"));
    mediaCat->addItem(i18n("Video"));
    mediaCat->setType(DB::Category::MediaTypeCategory);
    mediaCat->setShouldSave(false);
    // The media type is not stored in the media category,
    // but older versions of KPhotoAlbum stored a stub entry, which we need to remove first:
    if (m_db->m_categoryCollection.categoryForName(mediaCat->name()))
        m_db->m_categoryCollection.removeCategory(mediaCat->name());
    m_db->m_categoryCollection.addCategory(mediaCat);
}

void XMLDB::FileReader::loadCategories(ReaderPtr reader)
{
    static QString nameString = QString::fromUtf8("name");
    static QString iconString = QString::fromUtf8("icon");
    static QString viewTypeString = QString::fromUtf8("viewtype");
    static QString showString = QString::fromUtf8("show");
    static QString thumbnailSizeString = QString::fromUtf8("thumbnailsize");
    static QString positionableString = QString::fromUtf8("positionable");
    static QString metaString = QString::fromUtf8("meta");
    static QString tokensString = QString::fromUtf8("tokens");
    static QString valueString = QString::fromUtf8("value");
    static QString idString = QString::fromUtf8("id");
    static QString birthDateString = QString::fromUtf8("birthDate");
    static QString categoriesString = QString::fromUtf8("Categories");
    static QString categoryString = QString::fromUtf8("Category");
    static QString untaggedString = QString::fromUtf8("mark-untagged");

    ElementInfo info = reader->readNextStartOrStopElement(categoriesString);
    if (!info.isStartToken)
        reader->complainStartElementExpected(categoriesString);

    while (reader->readNextStartOrStopElement(categoryString).isStartToken) {
        const QString categoryName = unescape(reader->attribute(nameString));
        if (!categoryName.isNull()) {
            // Read Category info
            QString icon = reader->attribute(iconString);
            DB::Category::ViewType type = (DB::Category::ViewType)reader->attribute(viewTypeString, QString::fromLatin1("0")).toInt();
            int thumbnailSize = reader->attribute(thumbnailSizeString, QString::fromLatin1("32")).toInt();
            bool show = (bool)reader->attribute(showString, QString::fromLatin1("1")).toInt();
            bool positionable = (bool)reader->attribute(positionableString, QString::fromLatin1("0")).toInt();
            bool tokensCat = reader->attribute(metaString) == tokensString;

            DB::CategoryPtr cat = m_db->m_categoryCollection.categoryForName(categoryName);
            bool repairMode = false;
            if (cat) {
                DB::UserFeedback choice = m_db->uiDelegate().warningContinueCancel(
                    DB::LogMessage { DBLog(),
                                     QString::fromUtf8("Line %1, column %2: duplicate category '%3'")
                                         .arg(reader->lineNumber())
                                         .arg(reader->columnNumber())
                                         .arg(categoryName) },
                    i18n("<p>Line %1, column %2: duplicate category '%3'</p>"
                         "<p>Choose continue to ignore the duplicate category and try an automatic repair, "
                         "or choose cancel to quit.</p>",
                         reader->lineNumber(),
                         reader->columnNumber(),
                         categoryName),
                    i18n("Error in database file"));
                if (choice == DB::UserFeedback::Confirm)
                    repairMode = true;
                else
                    exit(-1);
            } else {
                cat = new DB::Category(categoryName, icon, type, thumbnailSize, show, positionable);
                if (tokensCat)
                    cat->setType(DB::Category::TokensCategory);
                m_db->m_categoryCollection.addCategory(cat);
            }

            // Read values
            QStringList items;
            QString untaggedTag;
            while (reader->readNextStartOrStopElement(valueString).isStartToken) {
                QString value = reader->attribute(valueString);
                if (reader->hasAttribute(idString)) {
                    int id = reader->attribute(idString).toInt();
                    if (id != 0) {
                        cat->setIdMapping(value, id);
                    } else {
                        if (useCompressedFileFormat()) {
                            qCWarning(DBLog) << "Tag" << categoryName << "/" << value << "has id=0!";
                            m_repairTagsWithNullIds = true;
                            cat->addZeroMapping(value);
                        }
                        // else just don't set the id mapping so that a new id gets assigned
                    }
                }
                if (reader->hasAttribute(birthDateString))
                    cat->setBirthDate(value, QDate::fromString(reader->attribute(birthDateString), Qt::ISODate));
                if (reader->hasAttribute(metaString) && reader->attribute(metaString) == untaggedString) {
                    untaggedTag = value;
                }
                items.append(value);
                reader->readEndElement();
            }
            if (repairMode) {
                // merge with duplicate category
                qCInfo(DBLog) << "Repairing category " << categoryName << ": merging items "
                              << cat->items() << " with " << items;
                items.append(cat->items());
                items.removeDuplicates();
            }
            cat->setItems(items);
            if (!untaggedTag.isEmpty()) {
                m_db->setUntaggedTag(cat->itemForName(untaggedTag));
            }
        }
    }

    createSpecialCategories();

    if (m_fileVersion < 7) {
        m_db->uiDelegate().information(
            DB::LogMessage { DBLog(), QString::fromLatin1("Standard category names are no longer used since index.xml "
                                                          "version 7. Standard categories will be left untranslated from now on.") },
            i18nc("Leave \"Folder\" and \"Media Type\" untranslated below, those will show up with "
                  "these exact names. Thanks :-)",
                  "<p><b>This version of KPhotoAlbum does not translate \"standard\" categories "
                  "any more.</b></p>"
                  "<p>This may mean that – if you use a locale other than English – some of your "
                  "categories are now displayed in English.</p>"
                  "<p>You can manually rename your categories any time and then save your database."
                  "</p>"
                  "<p>In some cases, you may get two additional empty categories, \"Folder\" and "
                  "\"Media Type\". You can delete those.</p>"),
            i18n("Changed standard category names"));
    }
}

void XMLDB::FileReader::loadImages(ReaderPtr reader)
{
    static QString fileString = QString::fromUtf8("file");
    static QString imagesString = QString::fromUtf8("images");
    static QString imageString = QString::fromUtf8("image");

    ElementInfo info = reader->readNextStartOrStopElement(imagesString);
    if (!info.isStartToken)
        reader->complainStartElementExpected(imagesString);

    while (reader->readNextStartOrStopElement(imageString).isStartToken) {
        const QString fileNameStr = reader->attribute(fileString);
        if (fileNameStr.isNull()) {
            qCWarning(DBLog, "Element did not contain a file attribute");
            return;
        }

        const DB::FileName dbFileName = DB::FileName::fromRelativePath(fileNameStr);

        DB::ImageInfoPtr info = load(dbFileName, reader);
        if (m_db->md5Map()->containsFile(dbFileName)) {
            if (m_db->md5Map()->contains(info->MD5Sum())) {
                qCWarning(DBLog) << "Merging duplicate entry for file" << dbFileName.relative();
                DB::ImageInfoPtr existingInfo = m_db->info(dbFileName);
                existingInfo->merge(*info);
            } else {
                m_db->uiDelegate().error(
                    DB::LogMessage { DBLog(), QString::fromUtf8("Conflicting information for file '%1': duplicate entry with different MD5 sum! Bailing out...").arg(dbFileName.relative()) },
                    i18n("<p>Line %1, column %2: duplicate entry for file '%3' with different MD5 sum.</p>"
                         "<p>Manual repair required!</p>",
                         reader->lineNumber(),
                         reader->columnNumber(),
                         dbFileName.relative()),
                    i18n("Error in database file"));
                exit(-1);
            }
        } else {
            m_db->m_images.append(info);
            m_db->m_md5map.insert(info->MD5Sum(), dbFileName);
        }
    }
}

void XMLDB::FileReader::loadBlockList(ReaderPtr reader)
{
    static QString fileString = QString::fromUtf8("file");
    static QString blockListString = QString::fromUtf8("blocklist");
    static QString blockString = QString::fromUtf8("block");

    ElementInfo info = reader->peekNext();
    if (info.isStartToken && info.tokenName == blockListString) {
        reader->readNextStartOrStopElement(blockListString);
        while (reader->readNextStartOrStopElement(blockString).isStartToken) {
            QString fileName = reader->attribute(fileString);
            if (!fileName.isEmpty())
                m_db->m_blockList.insert(DB::FileName::fromRelativePath(fileName));
            reader->readEndElement();
        }
    }
}

void XMLDB::FileReader::loadMemberGroups(ReaderPtr reader)
{
    static QString categoryString = QString::fromUtf8("category");
    static QString groupNameString = QString::fromUtf8("group-name");
    static QString memberString = QString::fromUtf8("member");
    static QString membersString = QString::fromUtf8("members");
    static QString memberGroupsString = QString::fromUtf8("member-groups");

    ElementInfo info = reader->peekNext();
    if (info.isStartToken && info.tokenName == memberGroupsString) {
        reader->readNextStartOrStopElement(memberGroupsString);
        while (reader->readNextStartOrStopElement(memberString).isStartToken) {
            QString category = reader->attribute(categoryString);

            QString group = reader->attribute(groupNameString);
            if (reader->hasAttribute(memberString)) {
                QString member = reader->attribute(memberString);
                m_db->m_members.addMemberToGroup(category, group, member);
            } else {
                const QStringList members = reader->attribute(membersString).split(QString::fromLatin1(","), Qt::SkipEmptyParts);
                for (const QString &memberItem : members) {
                    DB::CategoryPtr catPtr = m_db->m_categoryCollection.categoryForName(category);
                    if (!catPtr) { // category was not declared in "Categories"
                        qCWarning(DBLog) << "File corruption in index.xml. Inserting missing category: " << category;
                        catPtr = new DB::Category(category, QString::fromUtf8("dialog-warning"), DB::Category::TreeView, 32, false);
                        m_db->m_categoryCollection.addCategory(catPtr);
                    }
                    const QString member = catPtr->nameForId(memberItem.toInt());
                    if (member.isNull()) {
                        qCWarning(DBLog) << "Tag group" << category << "references non-existing tag with id"
                                         << memberItem << "!";
                        continue;
                    }
                    m_db->m_members.addMemberToGroup(category, group, member);
                }

                if (members.size() == 0) {
                    // Groups are stored even if they are empty, so we also have to read them.
                    // With no members, the above for loop will not be executed.
                    m_db->m_members.addGroup(category, group);
                }
            }

            reader->readEndElement();
        }
    }
}

/*
void XMLDB::FileReader::loadSettings(ReaderPtr reader)
{
    static QString settingsString = QString::fromUtf8("settings");
    static QString settingString = QString::fromUtf8("setting");
    static QString keyString = QString::fromUtf8("key");
    static QString valueString = QString::fromUtf8("value");

    ElementInfo info = reader->peekNext();
    if (info.isStartToken && info.tokenName == settingsString) {
        reader->readNextStartOrStopElement(settingString);
        while(reader->readNextStartOrStopElement(settingString).isStartToken) {
            if (reader->hasAttribute(keyString) && reader->hasAttribute(valueString)) {
                m_db->m_settings.insert(unescape(reader->attribute(keyString)),
                                        unescape(reader->attribute(valueString)));
            } else {
                qWarning() << "File corruption in index.xml. Setting either lacking a key or a "
                           << "value attribute. Ignoring this entry.";
            }
            reader->readEndElement();
        }
    }
}
*/

void XMLDB::FileReader::checkIfImagesAreSorted()
{
    if (m_db->uiDelegate().isDialogDisabled(QString::fromLatin1("checkWhetherImagesAreSorted")))
        return;

    Utilities::FastDateTime last(QDate(1900, 1, 1));
    bool wrongOrder = false;
    for (DB::ImageInfoListIterator it = m_db->m_images.begin(); !wrongOrder && it != m_db->m_images.end(); ++it) {
        if (last > (*it)->date().start() && (*it)->date().start().isValid())
            wrongOrder = true;
        last = (*it)->date().start();
    }

    if (wrongOrder) {
        m_db->uiDelegate().information(
            DB::LogMessage { DBLog(),
                             QString::fromLatin1("Database is not sorted by date.") },
            i18n("<p>Your images/videos are not sorted, which means that navigating using the date bar "
                 "will only work suboptimally.</p>"
                 "<p>In the <b>Maintenance</b> menu, you can find <b>Display Images with Incomplete Dates</b> "
                 "which you can use to find the images that are missing date information.</p>"
                 "<p>You can then select the images that you have reason to believe have a correct date "
                 "in either their Exif data or on the file, and execute <b>Maintenance->Read Exif Info</b> "
                 "to reread the information.</p>"
                 "<p>Finally, once all images have their dates set, you can execute "
                 "<b>Maintenance->Sort All by Date & Time</b> to sort them in the database. </p>"),
            i18n("Images/Videos Are Not Sorted"), QString::fromLatin1("checkWhetherImagesAreSorted"));
    }
}

void XMLDB::FileReader::checkIfAllImagesHaveSizeAttributes()
{
    if (m_db->uiDelegate().isDialogDisabled(QString::fromLatin1("checkWhetherAllImagesIncludesSize")))
        return;

    if (m_db->s_anyImageWithEmptySize) {
        m_db->uiDelegate().information(
            DB::LogMessage { DBLog(), QString::fromLatin1("Found image(s) without size information.") },
            i18n("<p>Not all the images in the database have information about image sizes; this is needed to "
                 "get the best result in the thumbnail view. To fix this, simply go to the <b>Maintenance</b> menu, "
                 "and first choose <b>Remove All Thumbnails</b>, and after that choose <tt>Build Thumbnails</tt>.</p>"
                 "<p>Not doing so will result in extra space around images in the thumbnail view - that is all - so "
                 "there is no urgency in doing it.</p>"),
            i18n("Not All Images Have Size Information"), QString::fromLatin1("checkWhetherAllImagesIncludesSize"));
    }
}

void XMLDB::FileReader::repairDB()
{
    if (m_repairTagsWithNullIds) {
        // the m_repairTagsWithNullIds is set in loadCategories()
        // -> care is taken so that multiple tags with id=0 all end up in the IdMap
        // afterwards, loadImages() applies fixes to the affected images
        // -> this happens in DB::ImageDB::possibleLoadCompressedCategories()
        // i.e. the zero ids still require cleanup:
        qCInfo(DBLog) << "Database contained tags with id=0 (possibly related to bug #415415). Assigning new ids for affected categories...";
        QString message = i18nc("repair merged tags",
                                "<p>Inconsistencies were found and repaired in your database. "
                                "Some categories now contain tags that were merged during the repair.</p>"
                                "<p>The following tags require manual inspection:"
                                "<ul>");
        QString logSummary = QString::fromLatin1("List of tags where manual inspection is required:\n");
        bool manualRepairNeeded = false;
        for (auto category : m_db->categoryCollection()->categories()) {
            QStringList tags = category->namesForIdZero();
            if (tags.size() > 1) {
                manualRepairNeeded = true;
                message += i18nc("repair merged tags", "<li>%1:<br/>", category->name());
                for (auto tagName : tags) {
                    message += i18nc("repair merged tags", "%1<br/>", tagName);
                    logSummary += QString::fromLatin1("%1/%2\n").arg(category->name(), tagName);
                }
                message += i18nc("repair merged tags", "</li>");
            }
            category->clearNullIds();
        }
        message += i18nc("repair merged tags",
                         "</ul></p>"
                         "<p>All affected images have also been marked with a tag "
                         "<em>KPhotoAlbum - manual repair needed</em>.</p>");
        if (manualRepairNeeded) {
            m_db->uiDelegate().information(DB::LogMessage { DBLog(), logSummary }, message, i18n("Database repair required"));
        }
    }
}

DB::ImageInfoPtr XMLDB::FileReader::load(const DB::FileName &fileName, ReaderPtr reader)
{
    DB::ImageInfoPtr info = DB::ImageDB::createImageInfo(fileName, reader, m_db);
    m_nextStackId = qMax(m_nextStackId, info->stackId() + 1);
    info->createFolderCategoryItem(m_folderCategory, m_db->m_members);
    return info;
}

XMLDB::ReaderPtr XMLDB::FileReader::readConfigFile(const QString &configFile)
{
    ReaderPtr reader = ReaderPtr(new XmlReader(m_db->uiDelegate(), configFile));
    QFile file(configFile);
    if (!file.exists()) {
        // Load a default setup
        QFile file(QStandardPaths::locate(QStandardPaths::DataLocation, QString::fromLatin1("default-setup")));
        if (!file.open(QIODevice::ReadOnly)) {
            m_db->uiDelegate().information(
                DB::LogMessage { DBLog(), QString::fromLatin1("default-setup not found in standard paths.") },
                i18n("<p>KPhotoAlbum was unable to load a default setup, which indicates an installation error</p>"
                     "<p>If you have installed KPhotoAlbum yourself, then you must remember to set the environment variable "
                     "<b>KDEDIRS</b>, to point to the topmost installation folder.</p>"
                     "<p>If you for example ran cmake with <b>-DCMAKE_INSTALL_PREFIX=/usr/local/kde</b>, then you must use the following "
                     "environment variable setup (this example is for Bash and compatible shells):</p>"
                     "<p><b>export KDEDIRS=/usr/local/kde</b></p>"
                     "<p>In case you already have KDEDIRS set, simply append the string as if you where setting the <b>PATH</b> "
                     "environment variable</p>"),
                i18n("No default setup file found"));
        } else {
            QTextStream stream(&file);
            stream.setCodec(QTextCodec::codecForName("UTF-8"));
            QString str = stream.readAll();

            // Replace the default setup's category and tag names with localized ones
            str = str.replace(QString::fromUtf8("People"), i18n("People"));
            str = str.replace(QString::fromUtf8("Places"), i18n("Places"));
            str = str.replace(QString::fromUtf8("Events"), i18n("Events"));
            str = str.replace(QString::fromUtf8("untagged"), i18n("untagged"));

            str = str.replace(QRegExp(QString::fromLatin1("imageDirectory=\"[^\"]*\"")), QString::fromLatin1(""));
            str = str.replace(QRegExp(QString::fromLatin1("htmlBaseDir=\"[^\"]*\"")), QString::fromLatin1(""));
            str = str.replace(QRegExp(QString::fromLatin1("htmlBaseURL=\"[^\"]*\"")), QString::fromLatin1(""));
            reader->addData(str);
        }
    } else {
        if (!file.open(QIODevice::ReadOnly)) {
            m_db->uiDelegate().error(
                DB::LogMessage { DBLog(), QString::fromLatin1("Unable to open '%1' for reading").arg(configFile) },
                i18n("Unable to open '%1' for reading", configFile), i18n("Error Running Demo"));
            exit(-1);
        }

        reader->addData(file.readAll());
#if 0
        QString errMsg;
        int errLine;
        int errCol;

        if ( !doc.setContent( &file, false, &errMsg, &errLine, &errCol )) {
            file.close();
            // If parsing index.xml fails let's see if we could use a backup instead
            Utilities::checkForBackupFile( configFile, i18n( "line %1 column %2 in file %3: %4", errLine , errCol , configFile , errMsg ) );
            if ( !file.open( QIODevice::ReadOnly ) || ( !doc.setContent( &file, false, &errMsg, &errLine, &errCol ) ) ) {
                KMessageBox::error( messageParent(), i18n( "Failed to recover the backup: %1", errMsg ) );
                exit(-1);
            }
        }
#endif
    }

    // Now read the content of the file.
#if 0
    QDomElement top = doc.documentElement();
    if ( top.isNull() ) {
        KMessageBox::error( messageParent(), i18n("Error in file %1: No elements found", configFile ) );
        exit(-1);
    }

    if ( top.tagName().toLower() != QString::fromLatin1( "kphotoalbum" ) &&
         top.tagName().toLower() != QString::fromLatin1( "kimdaba" ) ) { // KimDaBa compatibility
        KMessageBox::error( messageParent(), i18n("Error in file %1: expected 'KPhotoAlbum' as top element but found '%2'", configFile , top.tagName() ) );
        exit(-1);
    }
#endif

    file.close();
    return reader;
}

/**
 * @brief Unescape a string used as an XML attribute name.
 *
 * @see XMLDB::FileWriter::escape
 *
 * @param str the string to be unescaped
 * @return the unescaped string
 */
QString XMLDB::FileReader::unescape(const QString &str)
{
    static bool hashUsesCompressedFormat = useCompressedFileFormat();
    static QHash<QString, QString> s_cache;
    if (hashUsesCompressedFormat != useCompressedFileFormat())
        s_cache.clear();
    if (s_cache.contains(str))
        return s_cache[str];

    QString tmp(str);
    // Matches encoded characters in attribute names
    QRegExp rx(QString::fromLatin1("(_.)([0-9A-F]{2})"));
    int pos = 0;

    // Unencoding special characters if compressed XML is selected
    if (useCompressedFileFormat()) {
        while ((pos = rx.indexIn(tmp, pos)) != -1) {
            QString before = rx.cap(1) + rx.cap(2);
            QString after = QString::fromLatin1(QByteArray::fromHex(rx.cap(2).toLocal8Bit()));
            tmp.replace(pos, before.length(), after);
            pos += after.length();
        }
    } else
        tmp.replace(QString::fromLatin1("_"), QString::fromLatin1(" "));

    s_cache.insert(str, tmp);
    return tmp;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
