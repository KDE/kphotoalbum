/* Copyright (C) 2003-2015 Jesper K. Pedersen <blackie@kde.org>

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

// Qt includes
#include <QDebug>
#include <QDir>
#include <QTextCodec>
#include <QTextStream>
#include <QMap>
#include <QHash>
#include <QXmlStreamReader>
#include <QFile>
#include <QRegExp>

// KDE includes
#include <KConfigGroup>
#include <KCmdLineArgs>
#include <KLocale>
#include <KMessageBox>
#include <KStandardDirs>

// Local includes
#include "DB/MD5Map.h"
#include "Utilities/Util.h"
#include "MainWindow/DirtyIndicator.h"
#include "MainWindow/Window.h"
#include "Database.h"
#include "XMLCategory.h"
#include "CompressFileInfo.h"
#include "FileReader.h"

void XMLDB::FileReader::read( const QString& configFile )
{
    static QString versionString = QString::fromUtf8("version");
    static QString compressedString = QString::fromUtf8("compressed");

    ReaderPtr reader = readConfigFile( configFile );

    ElementInfo info = reader->readNextStartOrStopElement(QString::fromUtf8("KPhotoAlbum"));
    if (!info.isStartToken)
        reader->complainStartElementExpected(QString::fromUtf8("KPhotoAlbum"));

    m_fileVersion = reader->attribute( versionString, QString::fromLatin1( "1" ) ).toInt();

    if ( m_fileVersion > Database::fileVersion() ) {
        int ret = KMessageBox::warningContinueCancel( messageParent(),
                                                      i18n("<p>The database file (index.xml) is from a newer version of KPhotoAlbum!</p>"
                                                           "<p>Chances are you will be able to read this file, but when writing it back, "
                                                           "information saved in the newer version will be lost</p>"),
                                                      i18n("index.xml version mismatch"),
                                                      KStandardGuiItem::cont(), KStandardGuiItem::cancel(),
                                                      QString::fromLatin1( "checkDatabaseFileVersion" ) );
        if (ret == KStandardGuiItem::Cancel)
            exit(-1);
    }

    setUseCompressedFileFormat( reader->attribute(compressedString).toInt() );

    m_db->m_members.setLoading( true );
    loadCategories( reader );

    loadImages( reader );
    loadBlockList( reader );
    loadMemberGroups( reader );
    m_db->m_members.setLoading( false );

    checkIfImagesAreSorted();
    checkIfAllImagesHasSizeAttributes();
}


void XMLDB::FileReader::readTopNodeInConfigDocument( const QString& configFile, QDomElement top, QDomElement* options, QDomElement* images,
                                                     QDomElement* blockList, QDomElement* memberGroups )
{
    for ( QDomNode node = top.firstChild(); !node.isNull(); node = node.nextSibling() ) {
        if ( node.isElement() ) {
            QDomElement elm = node.toElement();
            QString tag = elm.tagName().toLower();
            if ( tag == QString::fromLatin1( "config" ) )
                ; // Skip for compatibility with 2.1 and older
            else if ( tag == QString::fromLatin1( "categories" ) || tag == QString::fromLatin1( "options" ) ) {
                // options is for KimDaBa 2.1 compatibility
                *options = elm;
            }
            else if ( tag == QString::fromLatin1( "configwindowsetup" ) )
                ; // Skip for compatibility with 2.1 and older
            else if ( tag == QString::fromLatin1("images") )
                *images = elm;
            else if ( tag == QString::fromLatin1( "blocklist" ) )
                *blockList = elm;
            else if ( tag == QString::fromLatin1( "member-groups" ) )
                *memberGroups = elm;
            else {
                KMessageBox::error( messageParent(),
                                    i18n("Error in file %1: unexpected element: '%2'", configFile , tag ) );
            }
        }
    }

    if ( options->isNull() )
        KMessageBox::sorry( messageParent(), i18n("Unable to find 'Options' tag in configuration file %1.", configFile ) );
    if ( images->isNull() )
        KMessageBox::sorry( messageParent(), i18n("Unable to find 'Images' tag in configuration file %1.", configFile ) );
}

void XMLDB::FileReader::createSpecialCategories()
{
    m_folderCategory = m_db->m_categoryCollection.categoryForName( QString::fromLatin1( "Folder" ) );
    if( m_folderCategory.isNull() ) {
        m_folderCategory = new XMLCategory( QString::fromLatin1("Folder"), QString::fromLatin1("folder"),
                                           DB::Category::TreeView, 32, false );
        m_db->m_categoryCollection.addCategory( m_folderCategory );
    }
    m_folderCategory->setSpecialCategory( true );
    dynamic_cast<XMLCategory*>( m_folderCategory.data() )->setShouldSave( false );

    DB::CategoryPtr tokenCat = m_db->m_categoryCollection.categoryForName( QString::fromLatin1( "Tokens" ) );
    if ( !tokenCat ) {
        tokenCat = new XMLCategory( QString::fromLatin1("Tokens"), QString::fromLatin1("flag-blue"),
                                    DB::Category::TreeView, 32, true );
        m_db->m_categoryCollection.addCategory( tokenCat );
    }
    tokenCat->setSpecialCategory( true );

    // KPhotoAlbum 2.2 did not write the tokens to the category section, so unless we do this small trick they
    // will not show up when importing.
    for ( char ch = 'A'; ch < 'Z'; ++ch )
        tokenCat->addItem( QString::fromLatin1("%1").arg( QChar::fromLatin1( ch) ) );

    DB::CategoryPtr mediaCat = m_db->m_categoryCollection.categoryForName( QString::fromLatin1( "Media Type" ) );
    if ( !mediaCat ) {
        mediaCat = new XMLCategory( QString::fromLatin1("Media Type"), QString::fromLatin1("video"),
                                    DB::Category::TreeView, 32, false );
        m_db->m_categoryCollection.addCategory( mediaCat );
    }
    mediaCat->addItem( QString::fromLatin1( "Image" ) );
    mediaCat->addItem( QString::fromLatin1( "Video" ) );
    mediaCat->setSpecialCategory( true );
    dynamic_cast<XMLCategory*>( mediaCat.data() )->setShouldSave( false );
}

void XMLDB::FileReader::loadCategories( ReaderPtr reader )
{
    static QString nameString = QString::fromUtf8("name");
    static QString iconString = QString::fromUtf8("icon");
    static QString viewTypeString = QString::fromUtf8("viewtype");
    static QString showString = QString::fromUtf8("show");
    static QString thumbnailSizeString = QString::fromUtf8("thumbnailsize");
    static QString positionableString = QString::fromUtf8("positionable");
    static QString valueString = QString::fromUtf8("value");
    static QString idString = QString::fromUtf8("id");
    static QString birthDateString = QString::fromUtf8("birthDate");
    static QString categoriesString = QString::fromUtf8("Categories");
    static QString categoryString = QString::fromUtf8("Category");


    ElementInfo info = reader->readNextStartOrStopElement(categoriesString);
    if (!info.isStartToken)
        reader->complainStartElementExpected(categoriesString);

    while ( reader->readNextStartOrStopElement(categoryString).isStartToken) {
        const QString categoryName = sanitizedCategoryName(unescape( reader->attribute(nameString) ));
        if ( !categoryName.isNull() )  {
            // Read Category info
            QString icon = reader->attribute(iconString);
            DB::Category::ViewType type =
                    (DB::Category::ViewType) reader->attribute( viewTypeString, QString::fromLatin1( "0" ) ).toInt();
            int thumbnailSize = reader->attribute( thumbnailSizeString, QString::fromLatin1( "32" ) ).toInt();
            bool show = (bool) reader->attribute( showString, QString::fromLatin1( "1" ) ).toInt();
            bool positionable = (bool) reader->attribute( positionableString, QString::fromLatin1( "0" ) ).toInt();

            DB::CategoryPtr cat = m_db->m_categoryCollection.categoryForName( categoryName );
            Q_ASSERT ( !cat );
            cat = new XMLCategory( categoryName, icon, type, thumbnailSize, show, positionable );
            m_db->m_categoryCollection.addCategory( cat );

            // Read values
            QStringList items;
            while( reader->readNextStartOrStopElement(valueString).isStartToken) {
                QString value = reader->attribute(valueString);
                if ( reader->hasAttribute(idString) ) {
                    int id = reader->attribute(idString).toInt();
                    static_cast<XMLCategory*>(cat.data())->setIdMapping( value, id );
                }
                if (reader->hasAttribute(birthDateString))
                    cat->setBirthDate(value,QDate::fromString(reader->attribute(birthDateString), Qt::ISODate));
                items.append( value );
                reader->readEndElement();
            }
            cat->setItems( items );
        }
    }

    createSpecialCategories();

    if (m_newToOldName.count() == 0) {
        // Normally, we end here. The rest only happens once, when the transition from dbv5 to
        // dbv6 is performed.
        return;
    }

    int ret = KMessageBox::warningContinueCancel(
        messageParent(),
        i18n("<p>This version of KPhotoAlbum will fix some issues with category names. This is "
             "a database internal update which won't affect the displayed category names or any "
             "tag data. Anyway, some files (category and tag thumbnails) have to be moved and the "
             "configuration file has to be fixed. If you want to know what exactly happens, read "
             "\"Differences to version 5\" in <kbd>documentation/database-layout.md</kbd>.</p>"
             "<p><b>Press \"Continue\" to run the update. This is what you normally want to do "
             "now.</b></p>"
             "<p>\"Cancel\" will skip the update. Only choose this if you don't want to or can't "
             "change any data now. You will be asked for the update on the next start again.</p>"),
        i18n("Database Update")
    );

    if (ret == KStandardGuiItem::Cancel) {
        QMessageBox::warning(
            messageParent(),
            i18n("Database Update"),
            i18n("<p><b>You skipped the database update!</b></p>"
                 "<p>Probably, you will miss some features in the running session (missing "
                 "category and tag thumbnails, broken \"untagged images\" feature, broken face "
                 "recognition). <b>Don't save your database or this will become permanent!</b></p>"
                 "<p>To run the update later, close KPhotoAlbum without saving the database and "
                 "don't use face recognition. You will be asked again for the update on the next "
                 "start.</p>")
        );
        MainWindow::Window::theMainWindow()->v6UpdateSkipped();
        return;
    }

    // Update the CategoryImages directory

    Settings::SettingsData* settings = Settings::SettingsData::instance();
    MainWindow::DirtyIndicator::markDirty();

    QDir dir(QString::fromUtf8("%1/CategoryImages").arg(settings->imageDirectory()));
    QMapIterator<QString, QString> oldToNew(m_newToOldName);

    while (oldToNew.hasNext()) {
        oldToNew.next();
        const QString &oldName = oldToNew.key();
        const QString &newName = oldToNew.value();

        if (oldName == newName) {
            continue;
        }

        // rename CategoryImages
        QStringList matchingFiles = dir.entryList(QStringList() << QString::fromUtf8("%1*").arg(newName));
        for (const QString &oldFileName : matchingFiles) {
            dir.rename(oldFileName, oldName + oldFileName.mid(newName.length()));
        }
    }

    // update category names for the Categories config
    KConfigGroup generalConfig = KGlobal::config()->group( QString::fromLatin1("General") );
    // Categories.untaggedCategory
    const QString untaggedCategory = QString::fromLatin1("untaggedCategory");
    QString untaggedCategoryValue = generalConfig.readEntry<QString>( untaggedCategory, QString());
    if ( !untaggedCategoryValue.isEmpty())
        generalConfig.writeEntry<QString>(untaggedCategory, sanitizedCategoryName(untaggedCategoryValue));
    // Categories.albumCategory
    const QString albumCategory = QString::fromLatin1("albumCategory");
    QString albumCategoryValue = generalConfig.readEntry<QString>( albumCategory, QString());
    if ( !albumCategoryValue.isEmpty())
        generalConfig.writeEntry<QString>(albumCategory, sanitizedCategoryName(albumCategoryValue));

    // update category names for privacy-lock settings
    KConfigGroup privacyConfig = KGlobal::config()->group( settings->groupForDatabase( "Privacy Settings" ));
    QStringList oldCategories = privacyConfig.readEntry<QStringList>( QString::fromLatin1("categories"), QStringList() );
    QStringList categories;
    for( QString &category : oldCategories ) {
        QString oldName = category;
        category = sanitizedCategoryName(oldName );
        categories << category;
        QString lockEntry = privacyConfig.readEntry<QString>(oldName, QString());
        if (! lockEntry.isEmpty() )
        {
            privacyConfig.writeEntry<QString>(category, lockEntry);
            privacyConfig.deleteEntry(oldName);
        }
    }
    privacyConfig.writeEntry<QStringList>( QString::fromLatin1("categories"), categories );

    MainWindow::Window::theMainWindow()->v6UpdateDone();
}

void XMLDB::FileReader::loadImages( ReaderPtr reader )
{
    static QString fileString = QString::fromUtf8("file");
    static QString imagesString = QString::fromUtf8("images");
    static QString imageString = QString::fromUtf8("image");

    ElementInfo info = reader->readNextStartOrStopElement(imagesString);
    if (!info.isStartToken)
        reader->complainStartElementExpected(imagesString);

    while (reader->readNextStartOrStopElement(imageString).isStartToken) {
        const QString fileNameStr = reader->attribute(fileString);
        if ( fileNameStr.isNull() ) {
            qWarning( "Element did not contain a file attribute" );
            return;
        }

        const DB::FileName dbFileName = DB::FileName::fromRelativePath(fileNameStr);

        DB::ImageInfoPtr info = load( dbFileName, reader );
        m_db->m_images.append(info);
        m_db->m_md5map.insert( info->MD5Sum(), dbFileName );
    }

}

void XMLDB::FileReader::loadBlockList( ReaderPtr reader )
{
    static QString fileString = QString::fromUtf8("file");
    static QString blockListString = QString::fromUtf8("blocklist");
    static QString blockString = QString::fromUtf8("block");

    ElementInfo info = reader->peekNext();
    if ( info.isStartToken && info.tokenName == blockListString ) {
        reader->readNextStartOrStopElement(blockListString);
        while (reader->readNextStartOrStopElement(blockString).isStartToken) {
            QString fileName = reader->attribute(fileString);
            if ( !fileName.isEmpty() )
                m_db->m_blockList << DB::FileName::fromRelativePath(fileName);
            reader->readEndElement();
        }
    }
}

void XMLDB::FileReader::loadMemberGroups( ReaderPtr reader )
{
    static QString categoryString = QString::fromUtf8("category");
    static QString groupNameString = QString::fromUtf8("group-name");
    static QString memberString = QString::fromUtf8("member");
    static QString membersString = QString::fromUtf8("members");
    static QString memberGroupsString = QString::fromUtf8("member-groups");

    ElementInfo info = reader->peekNext();
    if ( info.isStartToken && info.tokenName == memberGroupsString) {
        reader->readNextStartOrStopElement(memberGroupsString);
        while(reader->readNextStartOrStopElement(memberString).isStartToken) {
            QString category = sanitizedCategoryName(reader->attribute(categoryString));

            QString group = reader->attribute(groupNameString);
            if ( reader->hasAttribute(memberString) ) {
                QString member = reader->attribute(memberString);
                m_db->m_members.addMemberToGroup( category, group, member );
            }
            else {
                QStringList members = reader->attribute(membersString).split( QString::fromLatin1( "," ), QString::SkipEmptyParts );
                for( QStringList::Iterator membersIt = members.begin(); membersIt != members.end(); ++membersIt ) {
                    DB::CategoryPtr catPtr = m_db->m_categoryCollection.categoryForName( category );
                    if (catPtr.isNull())
                    { // category was not declared in "Categories"
                        qWarning() << "File corruption in index.xml. Inserting missing category: " << category;
                        catPtr = new XMLCategory(category, QString::fromUtf8("dialog-warning"), DB::Category::TreeView, 32, false);
                        m_db->m_categoryCollection.addCategory( catPtr );
                    }
                    XMLCategory* cat = static_cast<XMLCategory*>( catPtr.data() );
                    QString member = cat->nameForId( (*membersIt).toInt() );
                    if (member.isNull())
                        continue;
                    m_db->m_members.addMemberToGroup( category, group, member );
                }

                if(members.size() == 0) {
                    // Groups are stored even if they are empty, so we also have to read them.
                    // With no members, the above for loop will not be executed.
                    m_db->m_members.addGroup(category, group);
                }
            }

            reader->readEndElement();
        }
    }
}

void XMLDB::FileReader::checkIfImagesAreSorted()
{
    if ( !KMessageBox::shouldBeShownContinue( QString::fromLatin1( "checkWhetherImagesAreSorted" ) ) )
        return;

    QDateTime last( QDate( 1900, 1, 1 ) );
    bool wrongOrder = false;
    for( DB::ImageInfoListIterator it = m_db->m_images.begin(); !wrongOrder && it != m_db->m_images.end(); ++it ) {
        if ( last > (*it)->date().start() && (*it)->date().start().isValid() )
            wrongOrder = true;
        last = (*it)->date().start();
    }

    if ( wrongOrder ) {
        KMessageBox::information( messageParent(),
                          #ifdef HAVE_EXIV2
                                  i18n("<p>Your images/videos are not sorted, which means that navigating using the date bar "
                                       "will only work suboptimally.</p>"
                                       "<p>In the <b>Maintenance</b> menu, you can find <b>Display Images with Incomplete Dates</b> "
                                       "which you can use to find the images that are missing date information.</p>"
                                       "<p>You can then select the images that you have reason to believe have a correct date "
                                       "in either their EXIF data or on the file, and execute <b>Maintenance->Read EXIF Info</b> "
                                       "to reread the information.</p>"
                                       "<p>Finally, once all images have their dates set, you can execute "
                                       "<b>Maintenance->Sort All by Date & Time</b> to sort them in the database. </p>"),
                          #else
                                  i18n("<p>Your images/videos are not sorted, which means that navigating using the date bar "
                                       "will only work suboptimally.</p>"
                                       "<p>You also do not have EXIF support available, which means that you cannot read "
                                       "image dates from JPEG metadata. It is strongly recommended to recompile KPhotoAlbum "
                                       "with the <code>exiv2</code> library. After you have done so, you will be asked what "
                                       "to do to correct all the missing information.</p>"),
                          #endif
                                  i18n("Images/Videos Are Not Sorted"),
                                  QString::fromLatin1( "checkWhetherImagesAreSorted" ) );
    }

}

void XMLDB::FileReader::checkIfAllImagesHasSizeAttributes()
{
    QTime time;
    time.start();
    if ( !KMessageBox::shouldBeShownContinue( QString::fromLatin1( "checkWhetherAllImagesIncludesSize" ) ) )
        return;

    if ( m_db->s_anyImageWithEmptySize ) {
        KMessageBox::information( messageParent(),
                                  i18n("<p>Not all the images in the database have information about image sizes; this is needed to "
                                       "get the best result in the thumbnail view. To fix this, simply go to the <b>Maintenance</b> menu, "
                                       "and first choose <b>Remove All Thumbnails</b>, and after that choose <tt>Build Thumbnails</tt>.</p>"
                                       "<p>Not doing so will result in extra space around images in the thumbnail view - that is all - so "
                                       "there is no urgency in doing it.</p>"),
                                  i18n("Not All Images Have Size Information"),
                                  QString::fromLatin1( "checkWhetherAllImagesIncludesSize" ) );
    }

}

DB::ImageInfoPtr XMLDB::FileReader::load( const DB::FileName& fileName, ReaderPtr reader )
{
    DB::ImageInfoPtr info = XMLDB::Database::createImageInfo( fileName, reader, m_db, &m_newToOldName );
    m_nextStackId = qMax( m_nextStackId, info->stackId() + 1 );
    info->createFolderCategoryItem( m_folderCategory, m_db->m_members );
    return info;
}

XMLDB::ReaderPtr XMLDB::FileReader::readConfigFile( const QString& configFile )
{
    ReaderPtr reader = ReaderPtr(new XmlReader);
    QFile file( configFile );
    if ( !file.exists() ) {
        // Load a default setup
        QFile file(Utilities::locateDataFile(QString::fromLatin1("default-setup")));
        if ( !file.open( QIODevice::ReadOnly ) ) {
            KMessageBox::information( messageParent(),
                                      i18n( "<p>KPhotoAlbum was unable to load a default setup, which indicates an installation error</p>"
                                            "<p>If you have installed KPhotoAlbum yourself, then you must remember to set the environment variable "
                                            "<b>KDEDIRS</b>, to point to the topmost installation directory.</p>"
                                            "<p>If you for example ran cmake with <b>-DCMAKE_INSTALL_PREFIX=/usr/local/kde</b>, then you must use the following "
                                            "environment variable setup (this example is for Bash and compatible shells):</p>"
                                            "<p><b>export KDEDIRS=/usr/local/kde</b></p>"
                                            "<p>In case you already have KDEDIRS set, simply append the string as if you where setting the <b>PATH</b> "
                                            "environment variable</p>"), i18n("No default setup file found") );
        }
        else {
            QTextStream stream( &file );
            stream.setCodec( QTextCodec::codecForName("UTF-8") );
            QString str = stream.readAll();
            str = str.replace( QRegExp( QString::fromLatin1("imageDirectory=\"[^\"]*\"")), QString::fromLatin1("") );
            str = str.replace( QRegExp( QString::fromLatin1("htmlBaseDir=\"[^\"]*\"")), QString::fromLatin1("") );
            str = str.replace( QRegExp( QString::fromLatin1("htmlBaseURL=\"[^\"]*\"")), QString::fromLatin1("") );
            reader->addData(str);
        }
    }
    else {
        if ( !file.open( QIODevice::ReadOnly ) ) {
            KMessageBox::error( messageParent(), i18n("Unable to open '%1' for reading", configFile ), i18n("Error Running Demo") );
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

QString XMLDB::FileReader::unescape( const QString& str )
{
    static QHash<QString,QString> cache;
    if ( cache.contains(str) )
        return cache[str];

    QString tmp( str );
    // Matches encoded characters in attribute names
    QRegExp rx( QString::fromLatin1( "(_.)([0-9A-F]{2})" ) );
    int pos = 0;

    // Unencoding special characters if compressed XML is selected
    if ( useCompressedFileFormat() ) {
        while ( ( pos = rx.indexIn( tmp, pos ) ) != -1 ) {
            QString before = rx.cap( 1 ) + rx.cap( 2 );
            QString after = QString::fromLatin1( QByteArray::fromHex( rx.cap( 2 ).toLocal8Bit() ) );
            tmp.replace( pos, before.length(), after  );
            pos += after.length();
        }
    } else
        tmp.replace( QString::fromLatin1( "_" ), QString::fromLatin1( " " ) );

    cache.insert(str,tmp);
    return tmp;
}

QString XMLDB::FileReader::sanitizedCategoryName(const QString& category)
{
    // this fix only applies to older databases (<= version 5);
    // newer databases allow these categories, but without the "special meaning":
    if (m_fileVersion > 5) {
        return category;
    }

    QString mapped;
    // Silently correct some changes/bugs regarding category names
    // for a list of currently used category names, cf. DB::Category::standardCategories()
    if (category == QString::fromUtf8("Persons")) {
        // "Persons" is now "People"
        mapped = QString::fromUtf8("People");
    } else if (category == QString::fromUtf8("Locations")) {
        // "Locations" is now "Places"
        mapped = QString::fromUtf8("Places");
    } else {
        // Be sure to use the C locale category name for standard categories.
        // Older versions of KPA did store the localized category names.
        mapped = DB::Category::unLocalizedCategoryName(category);
    }

    m_newToOldName[mapped] = category;
    return mapped;
}

// TODO(hzeller): DEPENDENCY This pulls in the whole MainWindow dependency into the database backend.
QWidget *XMLDB::FileReader::messageParent()
{
    return MainWindow::Window::theMainWindow();
}

// vi:expandtab:tabstop=4 shiftwidth=4:
