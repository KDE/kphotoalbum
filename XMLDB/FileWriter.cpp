/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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

#include <klocale.h>
#include <kmessagebox.h>
#include <qfile.h>

#include "Database.h"
#include "MainWindow/Window.h"
#include "NumberedBackup.h"
#include "Utilities/List.h"
#include "XMLCategory.h"
#include <QXmlStreamWriter>
#include "ElementWriter.h"

using Utilities::StringSet;

static bool useCompressedBackup;

void XMLDB::FileWriter::save( const QString& fileName, bool isAutoSave )
{
    useCompressedBackup = Settings::SettingsData::instance()->useCompressedIndexXML();

    if ( !isAutoSave )
        NumberedBackup().makeNumberedBackup();

    // prepare XML document for saving:
    _db->_categoryCollection.initIdMap();
    QFile out(fileName + QString::fromAscii(".tmp"));
    if ( !out.open(QIODevice::WriteOnly | QIODevice::Text)) {
        KMessageBox::sorry( messageParent(),
                            i18n("<p>Could not save the image database to XML.</p>"
                                 "File %1 could not be opened because of the following error: %2"
                                 , out.fileName(), out.errorString() )
                            );
        return;
    }
    QXmlStreamWriter writer(&out);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();

    {
        ElementWriter dummy(writer, QString::fromLatin1("KPhotoAlbum"));
        writer.writeAttribute( QString::fromLatin1( "version" ), QString::fromLatin1( "3" ) );
        writer.writeAttribute( QString::fromLatin1( "compressed" ), QString::number(useCompressedBackup));

        saveCategories( writer );
        saveImages( writer );
        saveBlockList( writer );
        saveMemberGroups( writer );
    }

    // State: index.xml has previous DB version, index.xml.tmp has the current version.

    // original file can be safely deleted
    if ( ( ! QFile::remove( fileName ) ) && QFile::exists( fileName ) )
    {
        KMessageBox::sorry( messageParent(),
                            i18n("<p>Failed to remove old version of image database.</p>"
                                 "<p>Please try again or replace the file %1 with file %2 manually!</p>",
                                 fileName, out.fileName() )
                            );
        return;
    }
    // State: index.xml doesn't exist, index.xml.tmp has the current version.
    if ( ! out.rename( fileName ) )
    {
        KMessageBox::sorry( messageParent(),
                            i18n("<p>Failed to move temporary XML file to permanent location.</p>"
                                 "<p>Please try again or rename file %1 to %2 manually!</p>",
                                 out.fileName(), fileName )
                            );
        // State: index.xml.tmp has the current version.
        return;
    }
    // State: index.xml has the current version.
}

void XMLDB::FileWriter::saveCategories( QXmlStreamWriter& writer )
{
    QStringList categories = DB::ImageDB::instance()->categoryCollection()->categoryNames();
    {
        ElementWriter dummy(writer, QString::fromLatin1("Categories") );


        Q_FOREACH(const QString &name, categories) {
            DB::CategoryPtr category = DB::ImageDB::instance()->categoryCollection()->categoryForName( name );
            ElementWriter dummy(writer, QString::fromLatin1("Category") );
            writer.writeAttribute( QString::fromLatin1("name"), escape( name ) );

            writer.writeAttribute( QString::fromLatin1( "icon" ), category->iconName() );
            writer.writeAttribute( QString::fromLatin1( "show" ), QString::number(category->doShow()) );
            writer.writeAttribute( QString::fromLatin1( "viewtype" ), QString::number(category->viewType()));
            writer.writeAttribute( QString::fromLatin1( "thumbnailsize" ), QString::number(category->thumbnailSize()));

            if ( shouldSaveCategory( name ) ) {
                QStringList list =
                        Utilities::mergeListsUniqly(category->items(),
                                                    _db->_members.groups(name));

                Q_FOREACH(const QString &categoryName, list) {
                    ElementWriter dummy( writer, QString::fromLatin1("value") );
                    writer.writeAttribute( QString::fromLatin1("value"), categoryName );
                    writer.writeAttribute( QString::fromLatin1( "id" ),
                                           QString::number(static_cast<XMLCategory*>( category.data() )->idForName( categoryName ) ));
                }
            }
        }
    }
}

void XMLDB::FileWriter::saveImages( QXmlStreamWriter& writer )
{
    DB::ImageInfoList list = _db->_images;

    // Copy files from clipboard to end of overview, so we don't loose them
    Q_FOREACH(const DB::ImageInfoPtr &infoPtr, _db->_clipboard) {
        list.append(infoPtr);
    }

    {
        ElementWriter dummy(writer, QString::fromLatin1( "images" ) );

        Q_FOREACH(const DB::ImageInfoPtr &infoPtr, list) {
            save( writer, infoPtr );
        }
    }
}

void XMLDB::FileWriter::saveBlockList( QXmlStreamWriter& writer )
{
    ElementWriter dummy( writer, QString::fromLatin1( "blocklist" ) );
    Q_FOREACH(const DB::FileName &block, _db->_blockList) {
        ElementWriter dummy( writer,  QString::fromLatin1( "block" ) );
        writer.writeAttribute( QString::fromLatin1( "file" ), block.relative() );
    }
}

void XMLDB::FileWriter::saveMemberGroups( QXmlStreamWriter& writer )
{
    if ( _db->_members.isEmpty() )
        return;

    ElementWriter dummy( writer, QString::fromLatin1( "member-groups" ) );
    for( QMap< QString,QMap<QString,StringSet> >::ConstIterator memberMapIt= _db->_members.memberMap().constBegin();
         memberMapIt != _db->_members.memberMap().constEnd(); ++memberMapIt )
    {
        const QString categoryName = memberMapIt.key();
        if ( !shouldSaveCategory( categoryName ) )
            continue;

        QMap<QString,StringSet> groupMap = memberMapIt.value();
        for( QMap<QString,StringSet>::ConstIterator groupMapIt= groupMap.constBegin(); groupMapIt != groupMap.constEnd(); ++groupMapIt ) {
            StringSet members = groupMapIt.value();
            if ( useCompressedBackup ) {
                ElementWriter dummy( writer, QString::fromLatin1( "member" ) );
                writer.writeAttribute( QString::fromLatin1( "category" ), categoryName );
                writer.writeAttribute( QString::fromLatin1( "group-name" ), groupMapIt.key() );
                QStringList idList;
                Q_FOREACH(const QString& member, members) {
                    DB::CategoryPtr catPtr = _db->_categoryCollection.categoryForName( memberMapIt.key() );
                    XMLCategory* category = static_cast<XMLCategory*>( catPtr.data() );
                    idList.append( QString::number( category->idForName( member ) ) );
                }
                writer.writeAttribute( QString::fromLatin1( "members" ), idList.join( QString::fromLatin1( "," ) ) );
            }
            else {
                Q_FOREACH(const QString& member, members) {
                    ElementWriter dummy( writer,  QString::fromLatin1( "member" ) );
                    writer.writeAttribute( QString::fromLatin1( "category" ), memberMapIt.key() );
                    writer.writeAttribute( QString::fromLatin1( "group-name" ), groupMapIt.key() );
                    writer.writeAttribute( QString::fromLatin1( "member" ), member );
                }
            }
        }
    }
}

// This function will save an empty config element and a valid configWindowSetup element in the XML file.
// In versions of KPhotoAlbum newer than 2.1, this information is stored
// using KConfig, rather than in the database, so I need to add them like
// this to make the file readable by KPhotoAlbum 2.1.
void XMLDB::FileWriter::add21CompatXML( QDomElement& top )
{
    QDomDocument doc = top.ownerDocument();
    top.appendChild( doc.createElement( QString::fromLatin1( "config" ) ) );

    QByteArray conf = "<configWindowSetup>  <dock>   <name>Label and Dates</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </dock>  <dock>   <name>Image Preview</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </dock>  <dock>   <name>Description</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </dock>  <dock>   <name>Events</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </dock>  <dock>   <name>Places</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </dock>  <dock>   <name>People</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </dock>  <splitGroup>   <firstName>Label and Dates</firstName>   <secondName>Description</secondName>   <orientation>0</orientation>   <separatorPos>31</separatorPos>   <name>Label and Dates,Description</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </splitGroup>  <splitGroup>   <firstName>Label and Dates,Description</firstName>   <secondName>Image Preview</secondName>   <orientation>1</orientation>   <separatorPos>70</separatorPos>   <name>Label and Dates,Description,Image Preview</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </splitGroup>  <splitGroup>   <firstName>Places</firstName>   <secondName>Events</secondName>   <orientation>1</orientation>   <separatorPos>50</separatorPos>   <name>Places,Events</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </splitGroup>  <splitGroup>   <firstName>People</firstName>   <secondName>Places,Events</secondName>   <orientation>1</orientation>   <separatorPos>34</separatorPos>   <name>People,Places,Events</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </splitGroup>  <splitGroup>   <firstName>Label and Dates,Description,Image Preview</firstName>   <secondName>People,Places,Events</secondName>   <orientation>0</orientation>   <separatorPos>0</separatorPos>   <name>Label and Dates,Description,Image Preview,People,Places,Events</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </splitGroup>  <centralWidget>Label and Dates,Description,Image Preview,People,Places,Events</centralWidget>  <mainDockWidget>Label and Dates</mainDockWidget>  <geometry>   <x>6</x>   <y>6</y>   <width>930</width>   <height>492</height>  </geometry> </configWindowSetup>";

    QDomDocument tmpDoc;
    tmpDoc.setContent( conf );
    top.appendChild( tmpDoc.documentElement() );
}

void XMLDB::FileWriter::save( QXmlStreamWriter& writer, const DB::ImageInfoPtr& info )
{
    ElementWriter dummy( writer, QString::fromLatin1("image") );
    writer.writeAttribute( QString::fromLatin1("file"),  info->fileName().relative() );
    writer.writeAttribute( QString::fromLatin1("label"),  info->label() );
    if ( !info->description().isEmpty() )
        writer.writeAttribute( QString::fromLatin1("description"), info->description() );

    DB::ImageDate date = info->date();
    QDateTime start = date.start();
    QDateTime end = date.end();

    writer.writeAttribute( QString::fromLatin1( "startDate" ), start.toString(Qt::ISODate) );
    writer.writeAttribute( QString::fromLatin1( "endDate" ), end.toString(Qt::ISODate) );

    writer.writeAttribute( QString::fromLatin1("angle"),  QString::number(info->angle()));
    writer.writeAttribute( QString::fromLatin1( "md5sum" ), info->MD5Sum().toHexString() );
    writer.writeAttribute( QString::fromLatin1( "width" ), QString::number(info->size().width()));
    writer.writeAttribute( QString::fromLatin1( "height" ), QString::number(info->size().height()));

    if ( info->rating() != -1 ) {
        writer.writeAttribute( QString::fromLatin1("rating"), QString::number(info->rating()));
    }

    if ( info->stackId() ) {
        writer.writeAttribute( QString::fromLatin1("stackId"), QString::number(info->stackId()));
        writer.writeAttribute( QString::fromLatin1("stackOrder"), QString::number(info->stackOrder()));
    }

    const DB::GpsCoordinates& geoPos = info->geoPosition();
    if ( !geoPos.isNull() ) {
        writer.writeAttribute( QLatin1String("gpsPrec"), QString::number(geoPos.precision()));
        writer.writeAttribute( QLatin1String("gpsLon"), QString::number(geoPos.longitude()));
        writer.writeAttribute( QLatin1String("gpsLat"), QString::number(geoPos.latitude()));
        writer.writeAttribute( QLatin1String("gpsAlt"), QString::number(geoPos.altitude()));
    }

    if ( info->isVideo() )
        writer.writeAttribute( QLatin1String("videoLength"), QString::number(info->videoLength()));

    if ( useCompressedBackup )
        writeCategoriesCompressed( writer, info );
    else
        writeCategories( writer, info );
}

void XMLDB::FileWriter::writeCategories( QXmlStreamWriter& writer, const DB::ImageInfoPtr& info )
{
    ElementWriter topElm(writer, QString::fromLatin1("options"), false );

    QStringList grps = info->availableCategories();
    Q_FOREACH(const QString &name, grps) {
        if ( !shouldSaveCategory( name ) )
            continue;

        ElementWriter categoryElm(writer, QString::fromLatin1("option"), false );

        StringSet items = info->itemsOfCategory(name);
        if ( !items.isEmpty() ) {
            topElm.writeStartElement();
            categoryElm.writeStartElement();
            writer.writeAttribute( QString::fromLatin1("name"),  escape( name ) );
        }

        Q_FOREACH(const QString& itemValue, items) {
            ElementWriter dummy( writer, QString::fromLatin1("value") );
            writer.writeAttribute( QString::fromLatin1("value"), itemValue );
        }

    }
}

void XMLDB::FileWriter::writeCategoriesCompressed( QXmlStreamWriter& writer, const DB::ImageInfoPtr& info )
{
    QList<DB::CategoryPtr> categoryList = DB::ImageDB::instance()->categoryCollection()->categories();
    Q_FOREACH(const DB::CategoryPtr &category, categoryList) {
        QString categoryName = category->name();

        if ( !shouldSaveCategory( categoryName ) )
            continue;

        StringSet items = info->itemsOfCategory(categoryName);
        if ( !items.empty() ) {
            QStringList idList;
            Q_FOREACH(const QString &itemValue, items) {
                int id = static_cast<const XMLCategory*>(category.data())->idForName(itemValue);
                idList.append( QString::number( id ) );
            }
            writer.writeAttribute( escape( categoryName ), idList.join( QString::fromLatin1( "," ) ) );
        }
    }
}

bool XMLDB::FileWriter::shouldSaveCategory( const QString& categoryName ) const
{
    // Profiling indicated that this function was a hotspot, so this cache improved saving speed with 25%
    static QHash<QString,bool> cache;
    if ( cache.contains(categoryName))
        return cache[categoryName];

    // A few bugs has shown up, where an invalid category name has crashed KPA. I therefore checks for sauch invalid names here.
    if ( !_db->_categoryCollection.categoryForName( categoryName ) ) {
        qWarning("Invalid category name: %s", qPrintable(categoryName));
        cache.insert(categoryName,false);
        return false;
    }

    const bool shouldSave =  dynamic_cast<XMLCategory*>( _db->_categoryCollection.categoryForName( categoryName ).data() )->shouldSave();
    cache.insert(categoryName,shouldSave);
    return shouldSave;
}

QString XMLDB::FileWriter::escape( const QString& str )
{
    QString tmp( str );
    // Regex to match characters that are not allowed to start XML attribute names
    static const QRegExp rx( QString::fromLatin1( "([^a-zA-Z0-9:_])" ) );
    int pos = 0;

    // Encoding special characters if compressed XML is selected
    if ( useCompressedBackup ) {
        while ( ( pos = rx.indexIn( tmp, pos ) ) != -1 ) {
            QString before = rx.cap( 1 );
            QString after;
            after.sprintf( "_.%0X", rx.cap( 1 ).data()->toAscii());
            tmp.replace( pos, before.length(), after);
            pos += after.length();
        }
    } else
        tmp.replace( QString::fromLatin1( " " ), QString::fromLatin1( "_" ) );
    return tmp;
}

// TODO(hzeller): DEPENDENCY This pulls in the whole MainWindow dependency into the database backend.
QWidget *XMLDB::FileWriter::messageParent()
{
    return MainWindow::Window::theMainWindow();
}

// vi:expandtab:tabstop=4 shiftwidth=4:
