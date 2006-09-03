#include "FileWriter.h"
#include "Database.h"
#include "XMLCategory.h"
#include <kmessagebox.h>
#include "MainWindow/Window.h"
#include <klocale.h>
#include "NumberedBackup.h"
#include <kcmdlineargs.h>
#include <qfile.h>
#include "Utilities/Util.h"

void XMLDB::FileWriter::save( const QString& fileName, bool isAutoSave )
{
    if ( !isAutoSave )
        NumberedBackup().makeNumberedBackup();

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

    QFile out( fileName );

    if ( !out.open( IO_WriteOnly ) )
        KMessageBox::sorry( MainWindow::Window::theMainWindow(), i18n( "Could not open file '%1'." ).arg( fileName ) );
    else {
        QCString s = doc.toCString();
        out.writeBlock( s.data(), s.size()-1 );
        out.close();
    }
}

#include "FileWriter.h"
void XMLDB::FileWriter::saveCategories( QDomDocument doc, QDomElement top )
{
    QStringList categories = DB::ImageDB::instance()->categoryCollection()->categoryNames();
    QDomElement options;
    if ( KCmdLineArgs::parsedArgs()->isSet( "export-in-2.1-format" ) )
        options = doc.createElement( QString::fromLatin1("options") );
    else
        options = doc.createElement( QString::fromLatin1("Categories") );
    top.appendChild( options );


    for( QStringList::Iterator categoryIt = categories.begin(); categoryIt != categories.end(); ++categoryIt ) {
        const QString name = *categoryIt;
        DB::CategoryPtr category = DB::ImageDB::instance()->categoryCollection()->categoryForName( name );

        if ( !shouldSaveCategory( name ) )
            continue;

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

        QStringList list = category->items();
        list += _db->_members.groups(name);
        list = Utilities::removeDuplicates( list );

        for( QStringList::Iterator it2 = list.begin(); it2 != list.end(); ++it2 ) {
            QDomElement val = doc.createElement( QString::fromLatin1("value") );
            val.setAttribute( QString::fromLatin1("value"), *it2 );
            val.setAttribute( QString::fromLatin1( "id" ), static_cast<XMLCategory*>( category.data() )->idForName( *it2 ) );
            opt.appendChild( val );
        }
        options.appendChild( opt );
    }
}

void XMLDB::FileWriter::saveImages( QDomDocument doc, QDomElement top )
{
    DB::ImageInfoList list = _db->_images;

    // Copy files from clipboard to end of overview, so we don't loose them
    for( DB::ImageInfoListConstIterator it = _db->_clipboard.constBegin(); it != _db->_clipboard.constEnd(); ++it ) {
        list.append( *it );
    }

    QDomElement images = doc.createElement( QString::fromLatin1( "images" ) );
    top.appendChild( images );

    for( DB::ImageInfoListIterator it = list.begin(); it != list.end(); ++it ) {
        images.appendChild( save( doc, *it ) );
    }
}

void XMLDB::FileWriter::saveBlockList( QDomDocument doc, QDomElement top )
{
    QDomElement blockList = doc.createElement( QString::fromLatin1( "blocklist" ) );
    bool any=false;
    for( QStringList::Iterator it = _db->_blockList.begin(); it != _db->_blockList.end(); ++it ) {
        any=true;
        QDomElement elm = doc.createElement( QString::fromLatin1( "block" ) );
        elm.setAttribute( QString::fromLatin1( "file" ), *it );
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
    for( QMapConstIterator< QString,QMap<QString,StringSet> > memberMapIt= _db->_members.memberMap().begin();
         memberMapIt != _db->_members.memberMap().end(); ++memberMapIt )
    {
        const QString categoryName = memberMapIt.key();
        if ( !shouldSaveCategory( categoryName ) )
            continue;

        QMap<QString,StringSet> groupMap = memberMapIt.data();
        for( QMapIterator<QString,StringSet> groupMapIt= groupMap.begin(); groupMapIt != groupMap.end(); ++groupMapIt ) {
            StringSet members = groupMapIt.data();
            if ( Settings::SettingsData::instance()->useCompressedIndexXML() &&
                 !KCmdLineArgs::parsedArgs()->isSet( "export-in-2.1-format" )) {
                QDomElement elm = doc.createElement( QString::fromLatin1( "member" ) );
                elm.setAttribute( QString::fromLatin1( "category" ), categoryName );
                elm.setAttribute( QString::fromLatin1( "group-name" ), groupMapIt.key() );
                QStringList idList;
                for( StringSet::Iterator membersIt = members.begin(); membersIt != members.end(); ++membersIt ) {
                    DB::CategoryPtr catPtr = _db->_categoryCollection.categoryForName( memberMapIt.key() );
                    XMLCategory* category = static_cast<XMLCategory*>( catPtr.data() );
                    idList.append( QString::number( category->idForName( *membersIt ) ) );
                }
                elm.setAttribute( QString::fromLatin1( "members" ), idList.join( QString::fromLatin1( "," ) ) );
                memberNode.appendChild( elm );
            }
            else {
                for( StringSet::Iterator membersIt = members.begin(); membersIt != members.end(); ++membersIt ) {
                    QDomElement elm = doc.createElement( QString::fromLatin1( "member" ) );
                    memberNode.appendChild( elm );
                    elm.setAttribute( QString::fromLatin1( "category" ), memberMapIt.key() );
                    elm.setAttribute( QString::fromLatin1( "group-name" ), groupMapIt.key() );
                    elm.setAttribute( QString::fromLatin1( "member" ), *membersIt );
                }
            }
        }
    }

    top.appendChild( memberNode );
}

// This function will save an empty config element and a valid configWindowSetup element in the XML file.
// In versions of KPhotoAlbum newer than 2.1, these informations are stored
// using KConfig, rather than in the database, so I need to add them like
// this to make the file readable by KPhotoAlbum 2.1.
void XMLDB::FileWriter::add21CompatXML( QDomElement& top )
{
    QDomDocument doc = top.ownerDocument();
    top.appendChild( doc.createElement( QString::fromLatin1( "config" ) ) );

    QCString conf = QCString( "<configWindowSetup>  <dock>   <name>Label and Dates</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </dock>  <dock>   <name>Image Preview</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </dock>  <dock>   <name>Description</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </dock>  <dock>   <name>Keywords</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </dock>  <dock>   <name>Locations</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </dock>  <dock>   <name>Persons</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </dock>  <splitGroup>   <firstName>Label and Dates</firstName>   <secondName>Description</secondName>   <orientation>0</orientation>   <separatorPos>31</separatorPos>   <name>Label and Dates,Description</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </splitGroup>  <splitGroup>   <firstName>Label and Dates,Description</firstName>   <secondName>Image Preview</secondName>   <orientation>1</orientation>   <separatorPos>70</separatorPos>   <name>Label and Dates,Description,Image Preview</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </splitGroup>  <splitGroup>   <firstName>Locations</firstName>   <secondName>Keywords</secondName>   <orientation>1</orientation>   <separatorPos>50</separatorPos>   <name>Locations,Keywords</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </splitGroup>  <splitGroup>   <firstName>Persons</firstName>   <secondName>Locations,Keywords</secondName>   <orientation>1</orientation>   <separatorPos>34</separatorPos>   <name>Persons,Locations,Keywords</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </splitGroup>  <splitGroup>   <firstName>Label and Dates,Description,Image Preview</firstName>   <secondName>Persons,Locations,Keywords</secondName>   <orientation>0</orientation>   <separatorPos>0</separatorPos>   <name>Label and Dates,Description,Image Preview,Persons,Locations,Keywords</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </splitGroup>  <centralWidget>Label and Dates,Description,Image Preview,Persons,Locations,Keywords</centralWidget>  <mainDockWidget>Label and Dates</mainDockWidget>  <geometry>   <x>6</x>   <y>6</y>   <width>930</width>   <height>492</height>  </geometry> </configWindowSetup>" );

    QDomDocument tmpDoc;
    tmpDoc.setContent( conf );
    top.appendChild( tmpDoc.documentElement() );
}

QDomElement XMLDB::FileWriter::save( QDomDocument doc, const DB::ImageInfoPtr& info )
{
    QDomElement elm = doc.createElement( QString::fromLatin1("image") );
    elm.setAttribute( QString::fromLatin1("file"),  info->fileName( true ) );
    elm.setAttribute( QString::fromLatin1("label"),  info->label() );
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
    elm.setAttribute( QString::fromLatin1( "md5sum" ), info->MD5Sum() );
    elm.setAttribute( QString::fromLatin1( "width" ), info->size().width() );
    elm.setAttribute( QString::fromLatin1( "height" ), info->size().height() );

    if ( Settings::SettingsData::instance()->useCompressedIndexXML() && !KCmdLineArgs::parsedArgs()->isSet( "export-in-2.1-format" ) )
        writeCategoriesCompressed( elm, info );
    else
        writeCategories( doc, elm, info );

    info->drawList().save( doc, elm );
    return elm;
}

void XMLDB::FileWriter::writeCategories( QDomDocument doc, QDomElement top, const DB::ImageInfoPtr& info )
{
    QDomElement elm = doc.createElement( QString::fromLatin1("options") );

    bool anyAtAll = false;
    QStringList grps = info->availableCategories();
    for( QStringList::Iterator categoryIt = grps.begin(); categoryIt != grps.end(); ++categoryIt ) {
        QString name = *categoryIt;
        if ( !shouldSaveCategory( name ) )
            continue;

        QDomElement opt = doc.createElement( QString::fromLatin1("option") );
        opt.setAttribute( QString::fromLatin1("name"),  escape( name ) );

        StringSet items = info->itemsOfCategory(*categoryIt);
        bool any = false;
        for( StringSet::ConstIterator itemIt = items.begin(); itemIt != items.end(); ++itemIt ) {
            QDomElement val = doc.createElement( QString::fromLatin1("value") );
            val.setAttribute( QString::fromLatin1("value"), *itemIt );
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
    QValueList<DB::CategoryPtr> categoryList = DB::ImageDB::instance()->categoryCollection()->categories();
    for( QValueList<DB::CategoryPtr>::Iterator categoryIt = categoryList.begin(); categoryIt != categoryList.end(); ++categoryIt ) {
        QString categoryName = (*categoryIt)->name();

        if ( !shouldSaveCategory( categoryName ) )
            continue;

        StringSet items = info->itemsOfCategory(categoryName);
        if ( !items.isEmpty() ) {
            QStringList idList;
            for( StringSet::ConstIterator itemIt = items.begin(); itemIt != items.end(); ++itemIt ) {
                int id = static_cast<XMLCategory*>((*categoryIt).data())->idForName( *itemIt );
                idList.append( QString::number( id ) );
            }
            elm.setAttribute( escape( categoryName ), idList.join( QString::fromLatin1( "," ) ) );
        }
    }
}

bool XMLDB::FileWriter::shouldSaveCategory( const QString& categoryName ) const
{
    return dynamic_cast<XMLCategory*>( _db->_categoryCollection.categoryForName( categoryName ).data() )->shouldSave();
}

QString XMLDB::FileWriter::escape( const QString& str )
{
    QString tmp( str );
    tmp.replace( QString::fromLatin1( " " ), QString::fromLatin1( "_" ) );
    return tmp;
}

