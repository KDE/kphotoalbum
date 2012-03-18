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

#include <kcmdlineargs.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qfile.h>

#include "Database.h"
#include "MainWindow/Window.h"
#include "NumberedBackup.h"
#include "Utilities/List.h"
#include "XMLCategory.h"

using Utilities::StringSet;

void XMLDB::FileWriter::save( const QString& fileName, bool isAutoSave )
{
    if ( !isAutoSave )
        NumberedBackup().makeNumberedBackup();

    // prepare XML document for saving:
    _db->_categoryCollection.initIdMap();
    QDomDocument doc;

    doc.appendChild( doc.createProcessingInstruction( QString::fromLatin1("xml"), QString::fromLatin1("version=\"1.0\" encoding=\"UTF-8\"") ) );
    QDomElement top;
    if ( KCmdLineArgs::parsedArgs()->isSet( "export-in-2.1-format" ) ) {
        top = doc.createElement( QString::fromLatin1("KimDaBa") );
    }
    else {
        top = doc.createElement( QString::fromLatin1("KPhotoAlbum") );
        top.setAttribute( QString::fromLatin1( "version" ), QString::fromLatin1( "3" ) );
        top.setAttribute( QString::fromLatin1( "compressed" ), Settings::SettingsData::instance()->useCompressedIndexXML() );
    }
    doc.appendChild( top );

    if ( KCmdLineArgs::parsedArgs()->isSet( "export-in-2.1-format" ) )
        add21CompatXML( top );

    saveCategories( doc, top );
    saveImages( doc, top );
    saveBlockList( doc, top );
    saveMemberGroups( doc, top );

    // save XML document to temporary file:
    QFile out( fileName + QString::fromAscii(".tmp") );

    if ( !out.open( QIODevice::WriteOnly ) ) {
        KMessageBox::sorry( messageParent(), 
                i18n("<p>Could not save the image database to XML.</p>"
                    "File %1 could not be opened because of the following error: %2"
                    , out.fileName(), out.errorString() ) 
                );
        return;
    }
    else {
        QByteArray s = doc.toByteArray().append( '\n' );
        if ( ! ( out.write( s.data(), s.size()-1 ) == s.size()-1  && out.flush() ) )
        {
            KMessageBox::sorry( messageParent(), 
                    i18n("<p>Could not save the image database to XML.</p>"
                        "<p>File %1 could not be written because of the following error: %2</p>"
                        "<p>Your XML file still contains the previous version of the image database! "
                        "To avoid losing your modifications to the image database, try to fix "
                        "the mentioned error and then <b>save the file again.</b></p>"
                        , out.fileName(), out.errorString() ) 
                    );
            // clean up (damaged) temporary file:
            out.remove(); 
        } else {
            out.close();
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
    }
}

void XMLDB::FileWriter::saveCategories( QDomDocument doc, QDomElement top )
{
    QStringList categories = DB::ImageDB::instance()->categoryCollection()->categoryNames();
    QDomElement options;
    if ( KCmdLineArgs::parsedArgs()->isSet( "export-in-2.1-format" ) )
        options = doc.createElement( QString::fromLatin1("options") );
    else
        options = doc.createElement( QString::fromLatin1("Categories") );
    top.appendChild( options );


    Q_FOREACH(const QString &name, categories) {
        DB::CategoryPtr category = DB::ImageDB::instance()->categoryCollection()->categoryForName( name );

        QDomElement opt;
        if ( KCmdLineArgs::parsedArgs()->isSet( "export-in-2.1-format" ) )
            opt = doc.createElement( QString::fromLatin1("option") );
        else
            opt = doc.createElement( QString::fromLatin1("Category") );
        opt.setAttribute( QString::fromLatin1("name"), escape( name ) );

        opt.setAttribute( QString::fromLatin1( "icon" ), category->iconName() );
        opt.setAttribute( QString::fromLatin1( "show" ), category->doShow() );
        opt.setAttribute( QString::fromLatin1( "viewtype" ), category->viewType() );
        opt.setAttribute( QString::fromLatin1( "thumbnailsize" ), category->thumbnailSize() );

        if ( shouldSaveCategory( name ) ) {
            QStringList list =
                Utilities::mergeListsUniqly(category->items(),
                                            _db->_members.groups(name));

            Q_FOREACH(const QString &categoryName, list) {
                QDomElement val = doc.createElement( QString::fromLatin1("value") );
                val.setAttribute( QString::fromLatin1("value"), categoryName );
                val.setAttribute( QString::fromLatin1( "id" ), static_cast<XMLCategory*>( category.data() )->idForName( categoryName ) );
                opt.appendChild( val );
            }
        }
        options.appendChild( opt );
    }
}

void XMLDB::FileWriter::saveImages( QDomDocument doc, QDomElement top )
{
    DB::ImageInfoList list = _db->_images;

    // Copy files from clipboard to end of overview, so we don't loose them
    Q_FOREACH(const DB::ImageInfoPtr &infoPtr, _db->_clipboard) {
        list.append(infoPtr);
    }

    QDomElement images = doc.createElement( QString::fromLatin1( "images" ) );
    top.appendChild( images );

    Q_FOREACH(const DB::ImageInfoPtr &infoPtr, list) {
        images.appendChild( save( doc, infoPtr ) );
    }
}

void XMLDB::FileWriter::saveBlockList( QDomDocument doc, QDomElement top )
{
    QDomElement blockList = doc.createElement( QString::fromLatin1( "blocklist" ) );
    bool any=false;
    Q_FOREACH(const QString &block, _db->_blockList) {
        any=true;
        QDomElement elm = doc.createElement( QString::fromLatin1( "block" ) );
        elm.setAttribute( QString::fromLatin1( "file" ), block );
        blockList.appendChild( elm );
    }

    if (any)
        top.appendChild( blockList );
}

void XMLDB::FileWriter::saveMemberGroups( QDomDocument doc, QDomElement top )
{
    if ( _db->_members.isEmpty() )
        return;

    QDomElement memberNode = doc.createElement( QString::fromLatin1( "member-groups" ) );
    for( QMap< QString,QMap<QString,StringSet> >::ConstIterator memberMapIt= _db->_members.memberMap().constBegin();
         memberMapIt != _db->_members.memberMap().constEnd(); ++memberMapIt )
    {
        const QString categoryName = memberMapIt.key();
        if ( !shouldSaveCategory( categoryName ) )
            continue;

        QMap<QString,StringSet> groupMap = memberMapIt.value();
        for( QMap<QString,StringSet>::ConstIterator groupMapIt= groupMap.constBegin(); groupMapIt != groupMap.constEnd(); ++groupMapIt ) {
            StringSet members = groupMapIt.value();
            if ( Settings::SettingsData::instance()->useCompressedIndexXML() &&
                 !KCmdLineArgs::parsedArgs()->isSet( "export-in-2.1-format" )) {
                QDomElement elm = doc.createElement( QString::fromLatin1( "member" ) );
                elm.setAttribute( QString::fromLatin1( "category" ), categoryName );
                elm.setAttribute( QString::fromLatin1( "group-name" ), groupMapIt.key() );
                QStringList idList;
                Q_FOREACH(const QString& member, members) {
                    DB::CategoryPtr catPtr = _db->_categoryCollection.categoryForName( memberMapIt.key() );
                    XMLCategory* category = static_cast<XMLCategory*>( catPtr.data() );
                    idList.append( QString::number( category->idForName( member ) ) );
                }
                elm.setAttribute( QString::fromLatin1( "members" ), idList.join( QString::fromLatin1( "," ) ) );
                memberNode.appendChild( elm );
            }
            else {
                Q_FOREACH(const QString& member, members) {
                    QDomElement elm = doc.createElement( QString::fromLatin1( "member" ) );
                    memberNode.appendChild( elm );
                    elm.setAttribute( QString::fromLatin1( "category" ), memberMapIt.key() );
                    elm.setAttribute( QString::fromLatin1( "group-name" ), groupMapIt.key() );
                    elm.setAttribute( QString::fromLatin1( "member" ), member );
                }
            }
        }
    }

    top.appendChild( memberNode );
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

QDomElement XMLDB::FileWriter::save( QDomDocument doc, const DB::ImageInfoPtr& info )
{
    QDomElement elm = doc.createElement( QString::fromLatin1("image") );
    elm.setAttribute( QString::fromLatin1("file"),  info->fileName( DB::RelativeToImageRoot ) );
    elm.setAttribute( QString::fromLatin1("label"),  info->label() );
    if ( !info->description().isEmpty() )
        elm.setAttribute( QString::fromLatin1("description"), info->description() );

    DB::ImageDate date = info->date();
    QDateTime start = date.start();
    QDateTime end = date.end();

    if ( KCmdLineArgs::parsedArgs()->isSet( "export-in-2.1-format" ) ) {
        elm.setAttribute( QString::fromLatin1("yearFrom"), start.date().year() );
        elm.setAttribute( QString::fromLatin1("monthFrom"),  start.date().month() );
        elm.setAttribute( QString::fromLatin1("dayFrom"),  start.date().day() );
        elm.setAttribute( QString::fromLatin1("hourFrom"), start.time().hour() );
        elm.setAttribute( QString::fromLatin1("minuteFrom"), start.time().minute() );
        elm.setAttribute( QString::fromLatin1("secondFrom"), start.time().second() );

        elm.setAttribute( QString::fromLatin1("yearTo"), end.date().year() );
        elm.setAttribute( QString::fromLatin1("monthTo"),  end.date().month() );
        elm.setAttribute( QString::fromLatin1("dayTo"),  end.date().day() );

    }
    else {
        elm.setAttribute( QString::fromLatin1( "startDate" ), start.toString(Qt::ISODate) );
        elm.setAttribute( QString::fromLatin1( "endDate" ), end.toString(Qt::ISODate) );
    }

    elm.setAttribute( QString::fromLatin1("angle"),  info->angle() );
    elm.setAttribute( QString::fromLatin1( "md5sum" ), info->MD5Sum().toHexString() );
    elm.setAttribute( QString::fromLatin1( "width" ), info->size().width() );
    elm.setAttribute( QString::fromLatin1( "height" ), info->size().height() );

    if ( info->rating() != -1 ) {
        elm.setAttribute( QString::fromLatin1("rating"), info->rating() );
    }

    if ( info->stackId() ) {
        elm.setAttribute( QString::fromLatin1("stackId"), info->stackId() );
        elm.setAttribute( QString::fromLatin1("stackOrder"), info->stackOrder() );
    }

    const DB::GpsCoordinates& geoPos = info->geoPosition();
    if ( !geoPos.isNull() ) {
        elm.setAttribute( QLatin1String("gpsPrec"), geoPos.precision() );
        elm.setAttribute( QLatin1String("gpsLon"), geoPos.longitude() );
        elm.setAttribute( QLatin1String("gpsLat"), geoPos.latitude() );
        elm.setAttribute( QLatin1String("gpsAlt"), geoPos.altitude() );
    }

    if ( Settings::SettingsData::instance()->useCompressedIndexXML() && !KCmdLineArgs::parsedArgs()->isSet( "export-in-2.1-format" ) )
        writeCategoriesCompressed( elm, info );
    else
        writeCategories( doc, elm, info );

    return elm;
}

void XMLDB::FileWriter::writeCategories( QDomDocument doc, QDomElement top, const DB::ImageInfoPtr& info )
{
    QDomElement elm = doc.createElement( QString::fromLatin1("options") );

    bool anyAtAll = false;
    QStringList grps = info->availableCategories();
    Q_FOREACH(const QString &name, grps) {
        if ( !shouldSaveCategory( name ) )
            continue;

        QDomElement opt = doc.createElement( QString::fromLatin1("option") );
        opt.setAttribute( QString::fromLatin1("name"),  escape( name ) );

        StringSet items = info->itemsOfCategory(name);
        bool any = false;
        Q_FOREACH(const QString& itemValue, items) {
            QDomElement val = doc.createElement( QString::fromLatin1("value") );
            val.setAttribute( QString::fromLatin1("value"), itemValue );
            opt.appendChild( val );
            any = true;
            anyAtAll = true;
        }
        if ( any )
            elm.appendChild( opt );
    }

    if ( anyAtAll )
        top.appendChild( elm );
}

void XMLDB::FileWriter::writeCategoriesCompressed( QDomElement& elm, const DB::ImageInfoPtr& info )
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
            elm.setAttribute( escape( categoryName ), idList.join( QString::fromLatin1( "," ) ) );
        }
    }
}

bool XMLDB::FileWriter::shouldSaveCategory( const QString& categoryName ) const
{
    // A few bugs has shown up, where an invalid category name has crashed KPA. I therefore checks for sauch invalid names here.
    if ( !_db->_categoryCollection.categoryForName( categoryName ) ) {
        qWarning("Invalid category name: %s", qPrintable(categoryName));
        return false;
    }

    return dynamic_cast<XMLCategory*>( _db->_categoryCollection.categoryForName( categoryName ).data() )->shouldSave();
}

QString XMLDB::FileWriter::escape( const QString& str )
{
    QString tmp( str );
    // Regex to match characters that are not allowed to start XML attribute names
    QRegExp rx( QString::fromLatin1( "([^a-zA-Z0-9:_])" ) );
    int pos = 0;
    
    // Encoding special characters if compressed XML is selected
    if ( Settings::SettingsData::instance()->useCompressedIndexXML() && !KCmdLineArgs::parsedArgs()->isSet( "export-in-2.1-format" ) ) {
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
