/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "XMLDB.h"
#include "showbusycursor.h"
#include "options.h"
#include <qfileinfo.h>
#include <qfile.h>
#include <qdir.h>
#include <kmessagebox.h>
#include <klocale.h>
#include "util.h"
#include "groupCounter.h"
#include <qprogressdialog.h>
#include <qapplication.h>
#include <qeventloop.h>
#include "browser.h"
#include <qdict.h>
#include "mainview.h"
#include "imageinfo.h"
#include "imageinfoptr.h"
#include "categorycollection.h"
#include "XMLDB.moc"
#include <kstandarddirs.h>
#include <qregexp.h>
#include <stdlib.h>
#include "XMLCategory.h"
#include <ksharedptr.h>
#include "XMLImageDateRangeCollection.h"
#include <kcmdlineargs.h>
#include <kdebug.h>
#include "NumberedBackup.h"

bool XMLDB::XMLDB::_anyImageWithEmptySize = false;
XMLDB::XMLDB::XMLDB( const QString& configFile ) : _members( MemberMap( this ) )
{
    Util::checkForBackupFile( configFile );
    QDomElement top = readConfigFile( configFile );
    _fileVersion = top.attribute( QString::fromLatin1( "version" ), QString::fromLatin1( "1" ) ).toInt();
    QDomElement categories;
    QDomElement images;
    QDomElement blockList;
    QDomElement memberGroups;
    readTopNodeInConfigDocument( configFile, top, &categories, &images, &blockList, &memberGroups );

    loadCategories( categories );
    loadImages( images );
    loadBlockList( blockList );
    loadMemberGroups( memberGroups );

    checkIfImagesAreSorted();
    checkIfAllImagesHasSizeAttributes();
    checkAndWarnAboutVersionConflict();
}

int XMLDB::XMLDB::totalCount() const
{
    return _images.count();
}

QMap<QString,int> XMLDB::XMLDB::classify( const ImageSearchInfo& info, const QString &group )
{
    QMap<QString, int> map;
    GroupCounter counter( group );
    QDict<void> alreadyMatched = info.findAlreadyMatched( group );

    ImageSearchInfo noMatchInfo = info;
    QString currentMatchTxt = noMatchInfo.option( group );
    if ( currentMatchTxt.isEmpty() )
        noMatchInfo.setOption( group, ImageDB::NONE() );
    else
        noMatchInfo.setOption( group, QString::fromLatin1( "%1 & %2" ).arg(currentMatchTxt).arg(ImageDB::NONE()) );

    // Iterate through the whole database of images.
    for( ImageInfoListConstIterator it = _images.constBegin(); it != _images.constEnd(); ++it ) {
        bool match = !(*it)->isLocked() && info.match( *it ) && rangeInclude( *it );
        if ( match ) { // If the given image is currently matched.

            // Now iterate through all the categories the current image
            // contains, and increase them in the map mapping from category
            // to count.
            QStringList list = (*it)->itemsOfCategory(group);
            counter.count( list );
            for( QStringList::Iterator it2 = list.begin(); it2 != list.end(); ++it2 ) {
                if ( !alreadyMatched[*it2] ) // We do not want to match "Jesper & Jesper"
                    map[*it2]++;
            }

            // Find those with no other matches
            if ( noMatchInfo.match( *it ) )
                map[ImageDB::NONE()]++;
        }
    }

    QMap<QString,int> groups = counter.result();
    for( QMapIterator<QString,int> it= groups.begin(); it != groups.end(); ++it ) {
        map[it.key()] = it.data();
    }

    return map;
}

void XMLDB::XMLDB::renameCategory( const QString& oldName, const QString newName )
{
    for( ImageInfoListIterator it = _images.begin(); it != _images.end(); ++it ) {
        (*it)->renameCategory( oldName, newName );
    }
}

void XMLDB::XMLDB::addToBlockList( const QStringList& list )
{
    for( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
        ImageInfoPtr inf= info(*it);
        _blockList << inf->fileName( true );
        _images.remove( inf );
    }
    emit totalChanged( _images.count() );
}

void XMLDB::XMLDB::deleteList( const QStringList& list )
{
    for( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
        ImageInfoPtr inf= info(*it);
        _images.remove( inf );
    }
    emit totalChanged( _images.count() );
}

void XMLDB::XMLDB::renameItem( Category* category, const QString& oldName, const QString& newName )
{
    for( ImageInfoListIterator it = _images.begin(); it != _images.end(); ++it ) {
        (*it)->renameItem( category->name(), oldName, newName );
    }
}

void XMLDB::XMLDB::deleteItem( Category* category, const QString& option )
{
    for( ImageInfoListIterator it = _images.begin(); it != _images.end(); ++it ) {
        (*it)->removeOption( category->name(), option );
    }
}

void XMLDB::XMLDB::lockDB( bool lock, bool exclude  )
{
    ImageSearchInfo info = Options::instance()->currentLock();
    for( ImageInfoListIterator it = _images.begin(); it != _images.end(); ++it ) {
        if ( lock ) {
            bool match = info.match( *it );
            if ( !exclude )
                match = !match;
            (*it)->setLocked( match );
        }
        else
            (*it)->setLocked( false );
    }
}


void XMLDB::XMLDB::addImages( const ImageInfoList& images )
{
    ImageInfoList newImages = images.sort();
    if ( _images.count() == 0 ) {
        // case 1: The existing imagelist is empty.
        _images = newImages;
    }
    else if ( newImages.count() == 0 ) {
        // case 2: No images to merge in - that's easy ;-)
    }
    else if ( newImages.first()->date().start() > _images.last()->date().start() ) {
        // case 2: The new list is later than the existsing
        _images.appendList(newImages);
    }
    else if ( _images.isSorted() ) {
        // case 3: The lists overlaps, and the existsing list is sorted
        _images.mergeIn( newImages );
    }
    else{
        // case 4: The lists overlaps, and the existsing list is not sorted in the overlapping range.
        _images.appendList( newImages );
    }
    emit totalChanged( _images.count() );
    emit dirty();
}

ImageInfoPtr XMLDB::XMLDB::info( const QString& fileName ) const
{
    static QMap<QString, ImageInfoPtr > fileMap;

    if ( fileMap.contains( fileName ) )
        return fileMap[ fileName ];
    else {
        fileMap.clear();
        for( ImageInfoListConstIterator it = _images.constBegin(); it != _images.constEnd(); ++it ) {
            fileMap.insert( (*it)->fileName(), *it );
        }
        if ( fileMap.contains( fileName ) )
            return fileMap[ fileName ];
    }
    return 0;
}

void XMLDB::XMLDB::checkIfImagesAreSorted()
{
    if ( !KMessageBox::shouldBeShownContinue( QString::fromLatin1( "checkWhetherImagesAreSorted" ) ) )
        return;

    QDateTime last( QDate( 1900, 1, 1 ) );
    bool wrongOrder = false;
    for( ImageInfoListIterator it = _images.begin(); !wrongOrder && it != _images.end(); ++it ) {
        if ( last > (*it)->date().start() )
            wrongOrder = true;
        last = (*it)->date().start();
    }

    if ( wrongOrder ) {
        KMessageBox::information( MainView::theMainView(),
                                  i18n("<qt><p>Your images are not sorted, which means that navigating using the date bar "
                                       "will only work suboptimally.</p>"
                                       "<p>In the <b>Maintenance</b> menu, you can find <b>Display Images with Incomplete Dates</b> "
                                       "which you can use to find the images that are missing date information.</p>"
                                       "You can then select the images that you have reason to believe have a correct date "
                                       "in either their EXIF data or on the file, and execute <b>Maintainance->Read EXIF Info</b> "
                                       "to reread the information.</p>"
                                       "<p>Finally, once all images have their dates set, you can execute "
                                       "<b>Images->Sort Selected by Date & Time</b> to sort them in the database.</p></qt>"),
                                  i18n("Images Are Not Sorted"),
                                  QString::fromLatin1( "checkWhetherImagesAreSorted" ) );
    }
}

bool XMLDB::XMLDB::rangeInclude( ImageInfoPtr info ) const
{
    if (_selectionRange.date().start().isNull() )
        return true;

    ImageDateRange::MatchType tp = info->dateRange().isIncludedIn( _selectionRange );
    if ( _includeFuzzyCounts )
        return ( tp == ImageDateRange::ExactMatch || tp == ImageDateRange::RangeMatch );
    else
        return ( tp == ImageDateRange::ExactMatch );
}

void XMLDB::XMLDB::checkIfAllImagesHasSizeAttributes()
{
    QTime time;
    time.start();
    if ( !KMessageBox::shouldBeShownContinue( QString::fromLatin1( "checkWhetherAllImagesIncludesSize" ) ) )
        return;

    if ( _anyImageWithEmptySize ) {
        KMessageBox::information( MainView::theMainView(),
                                  i18n("<qt><p>Not all the images in the database have information about image sizes; this is needed to "
                                       "get the best result in the thumbnail view. To fix this, simply go to the <tt>Maintainance</tt> menu, and first "
                                       "choose <tt>Remove All Thumbnails</tt>, and after that choose <tt>Build Thumbnails</tt>.</p>"
                                       "<p>Not doing so will result in extra space around images in the thumbnail view - that is all - so "
                                       "there is no urgency in doing it.</p></qt>"),
                                  i18n("Not All Images Have Size Information"),
                                  QString::fromLatin1( "checkWhetherAllImagesIncludesSize" ) );
    }
}


const MemberMap& XMLDB::XMLDB::memberMap()
{
    return _members;
}

void XMLDB::XMLDB::setMemberMap( const MemberMap& members )
{
    _members = members;
}

void XMLDB::XMLDB::loadCategories( const QDomElement& elm )
{
    createSpecialCategories();
    Q_ASSERT( elm.tagName() == QString::fromLatin1( "options" ) );

    for ( QDomNode nodeOption = elm.firstChild(); !nodeOption.isNull(); nodeOption = nodeOption.nextSibling() )  {

        if ( nodeOption.isElement() )  {
            QDomElement elmOption = nodeOption.toElement();
            Q_ASSERT( elmOption.tagName() == QString::fromLatin1("option") );
            QString name = elmOption.attribute( QString::fromLatin1("name") );

            if ( !name.isNull() )  {
                // Read Category info
                QString icon= elmOption.attribute( QString::fromLatin1("icon") );
                Category::ViewSize size =
                    (Category::ViewSize) elmOption.attribute( QString::fromLatin1("viewsize"), QString::fromLatin1( "0" ) ).toInt();
                Category::ViewType type =
                    (Category::ViewType) elmOption.attribute( QString::fromLatin1("viewtype"), QString::fromLatin1( "0" ) ).toInt();
                bool show = (bool) elmOption.attribute( QString::fromLatin1( "show" ),
                                                        QString::fromLatin1( "1" ) ).toInt();

                CategoryPtr cat = _categoryCollection.categoryForName( name );
                if ( !cat ) {
                    // Special categories are already created so they
                    // should not be created here. Besides they are not
                    // configurable (they were in previous versions, but
                    // that is just too much trouble for too litle gain)
                    cat = new XMLCategory( name, icon, size, type, show );
                    _categoryCollection.addCategory( cat );
                }

                // Read values
                QStringList items;
                for ( QDomNode nodeValue = elmOption.firstChild(); !nodeValue.isNull();
                      nodeValue = nodeValue.nextSibling() ) {
                    if ( nodeValue.isElement() ) {
                        QDomElement elmValue = nodeValue.toElement();
                        Q_ASSERT( elmValue.tagName() == QString::fromLatin1("value") );
                        QString value = elmValue.attribute( QString::fromLatin1("value") );
                        if ( elmValue.hasAttribute( QString::fromLatin1( "id" ) ) ) {
                            int id = elmValue.attribute( QString::fromLatin1( "id" ) ).toInt();
                            static_cast<XMLCategory*>(cat.data())->setIdMapping( value, id );
                        }
                        items.append( value );
                    }
                }
                cat->setItems( items );
            }
        }
    }
}

void XMLDB::XMLDB::createSpecialCategories()
{
    CategoryPtr folderCat = _categoryCollection.categoryForName( QString::fromLatin1( "Folder" ) );
    if( folderCat == 0 ) {
        folderCat = new XMLCategory( QString::fromLatin1("Folder"), QString::fromLatin1("folder"),
                                     Category::Small, Category::ListView, false );
        _categoryCollection.addCategory( folderCat );
    }
    folderCat->setSpecialCategory( true );


    CategoryPtr tokenCat = _categoryCollection.categoryForName( QString::fromLatin1( "Tokens" ) );
    if ( !tokenCat ) {
        tokenCat = new XMLCategory( QString::fromLatin1("Tokens"), QString::fromLatin1("cookie"),
                                    Category::Small, Category::ListView, true );
        _categoryCollection.addCategory( tokenCat );
    }
    tokenCat->setSpecialCategory( true );
}

void XMLDB::XMLDB::save( const QString& fileName, bool isAutoSave )
{
    if ( !isAutoSave )
        NumberedBackup().makeNumberedBackup();

    _categoryCollection.initIdMap();
    QDomDocument doc;

    doc.appendChild( doc.createProcessingInstruction( QString::fromLatin1("xml"), QString::fromLatin1("version=\"1.0\" encoding=\"UTF-8\"") ) );
    QDomElement top = doc.createElement( QString::fromLatin1("KimDaBa") );
    top.setAttribute( QString::fromLatin1( "version" ), QString::fromLatin1( "2" ) );
    doc.appendChild( top );

    if ( KCmdLineArgs::parsedArgs()->isSet( "export-in-2.1-format" ) )
        add21CompatXML( top );

    saveCategories( doc, top );
    saveImages( doc, top );
    saveBlockList( doc, top );
    saveMemberGroups( doc, top );

    QFile out( fileName );

    if ( !out.open( IO_WriteOnly ) )
        KMessageBox::sorry( MainView::theMainView(), i18n( "Could not open file '%1'." ).arg( fileName ) );
    else {
        QCString s = doc.toCString();
        out.writeBlock( s.data(), s.size()-1 );
        out.close();
    }
}


MD5Map* XMLDB::XMLDB::md5Map()
{
    return &_md5map;
}

bool XMLDB::XMLDB::isBlocking( const QString& fileName )
{
    return _blockList.contains( fileName );
}

QDomElement XMLDB::XMLDB::readConfigFile( const QString& configFile )
{
    QDomDocument doc;
    QFile file( configFile );
    if ( !file.exists() ) {
        // Load a default setup
        QFile file( locate( "data", QString::fromLatin1( "kimdaba/default-setup" ) ) );
        if ( !file.open( IO_ReadOnly ) ) {
            KMessageBox::information( 0, i18n( "<qt><p>KimDaBa was unable to load a default setup, which indicates an installation error</p>"
                                               "<p>If you have installed KimDaBa yourself, then you must remember to set the environment variable "
                                               "<b>KDEDIRS</b>, to point to the topmost installation directory.</p>"
                                               "<p>If you for example ran configure with <tt>--prefix=/usr/local/kde</tt>, then you must use the following "
                                               "environment variable setup (this example is for Bash and compatible shells):</p>"
                                               "<p><b>export KDEDIRS=/usr/local/kde</b></p>"
                                               "<p>In case you already have KDEDIRS set, simply append the string as if you where setting the <b>PATH</b> "
                                               "environment variable</p></qt>"), i18n("No default setup file found") );
        }
        else {
            QTextStream stream( &file );
            stream.setEncoding( QTextStream::UnicodeUTF8 );
            QString str = stream.read();
            str = str.replace( QString::fromLatin1( "Persons" ), i18n( "Persons" ) );
            str = str.replace( QString::fromLatin1( "Locations" ), i18n( "Locations" ) );
            str = str.replace( QString::fromLatin1( "Keywords" ), i18n( "Keywords" ) );
            str = str.replace( QRegExp( QString::fromLatin1("imageDirectory=\"[^\"]*\"")), QString::fromLatin1("") );
            str = str.replace( QRegExp( QString::fromLatin1("htmlBaseDir=\"[^\"]*\"")), QString::fromLatin1("") );
            str = str.replace( QRegExp( QString::fromLatin1("htmlBaseURL=\"[^\"]*\"")), QString::fromLatin1("") );
            doc.setContent( str );
        }
    }
    else {
        if ( !file.open( IO_ReadOnly ) ) {
            KMessageBox::error( MainView::theMainView(), i18n("Unable to open '%1' for reading").arg( configFile ), i18n("Error Running Demo") );
            exit(-1);
        }

        QString errMsg;
        int errLine;
        int errCol;

        if ( !doc.setContent( &file, false, &errMsg, &errLine, &errCol )) {
            KMessageBox::error( MainView::theMainView(), i18n("Error on line %1 column %2 in file %3: %4").arg( errLine ).arg( errCol ).arg( configFile ).arg( errMsg ) );
            exit(-1);
        }
    }

    // Now read the content of the file.
    QDomElement top = doc.documentElement();
    if ( top.isNull() ) {
        KMessageBox::error( MainView::theMainView(), i18n("Error in file %1: No elements found").arg( configFile ) );
        exit(-1);
    }

    if ( top.tagName().lower() != QString::fromLatin1( "kimdaba" ) ) {
        KMessageBox::error( MainView::theMainView(), i18n("Error in file %1: expected 'KimDaBa' as top element but found '%2'").arg( configFile ).arg( top.tagName() ) );
        exit(-1);
    }

    file.close();
    return top;
}

void XMLDB::XMLDB::readTopNodeInConfigDocument( const QString& configFile, QDomElement top, QDomElement* options, QDomElement* images,
                                         QDomElement* blockList, QDomElement* memberGroups )
{
    for ( QDomNode node = top.firstChild(); !node.isNull(); node = node.nextSibling() ) {
        if ( node.isElement() ) {
            QDomElement elm = node.toElement();
            QString tag = elm.tagName().lower();
            if ( tag == QString::fromLatin1( "config" ) )
                ; // Skip for compatibility with 2.1 and older
            else if ( tag == QString::fromLatin1( "options" ) )
                *options = elm;
            else if ( tag == QString::fromLatin1( "configwindowsetup" ) )
                ; // Skip for compatibility with 2.1 and older
            else if ( tag == QString::fromLatin1("images") )
                *images = elm;
            else if ( tag == QString::fromLatin1( "blocklist" ) )
                *blockList = elm;
            else if ( tag == QString::fromLatin1( "member-groups" ) )
                *memberGroups = elm;
            else {
                KMessageBox::error( MainView::theMainView(),
                                    i18n("Error in file %1: unexpected element: '%2*").arg( configFile ).arg( tag ) );
            }
        }
    }

    if ( options->isNull() )
        KMessageBox::sorry( MainView::theMainView(), i18n("Unable to find 'Options' tag in configuration file %1.").arg( configFile ) );
    if ( images->isNull() )
        KMessageBox::sorry( MainView::theMainView(), i18n("Unable to find 'Images' tag in configuration file %1.").arg( configFile ) );
}

void XMLDB::XMLDB::loadImages( const QDomElement& images )
{
    QString directory = Options::instance()->imageDirectory();

    for ( QDomNode node = images.firstChild(); !node.isNull(); node = node.nextSibling() )  {
        QDomElement elm;
        if ( node.isElement() )
            elm = node.toElement();
        else
            continue;

        QString fileName = elm.attribute( QString::fromLatin1("file") );
        if ( fileName.isNull() )
            qWarning( "Element did not contain a file attribute" );
        else {
            ImageInfoPtr info = load( fileName, elm );
            _images.append(info);
            _md5map.insert( info->MD5Sum(), fileName );
        }
    }

}

ImageInfoPtr XMLDB::XMLDB::load( const QString& fileName, QDomElement elm )
{
    ImageInfoPtr info = createImageInfo( fileName, elm, this );
    // This is for compatibility with KimDaBa 2.1 where this info was not saved.
    QString folderName = Util::relativeFolderName( fileName );
    info->setOption( QString::fromLatin1( "Folder") , QStringList( folderName ) );
    _categoryCollection.categoryForName(QString::fromLatin1("Folder"))->addItem( folderName );
    return info;
}

void XMLDB::XMLDB::loadBlockList( const QDomElement& blockList )
{
    for ( QDomNode node = blockList.firstChild(); !node.isNull(); node = node.nextSibling() )  {
        QDomElement elm;
        if ( node.isElement() )
            elm = node.toElement();
        else
            continue;

        QString fileName = elm.attribute( QString::fromLatin1( "file" ) );
        if ( !fileName.isEmpty() )
            _blockList << fileName;
    }
}

void XMLDB::XMLDB::loadMemberGroups( const QDomElement& memberGroups )
{
    for ( QDomNode node = memberGroups.firstChild(); !node.isNull(); node = node.nextSibling() ) {
        if ( node.isElement() ) {
            QDomElement elm = node.toElement();
            QString category = elm.attribute( QString::fromLatin1( "category" ) );
            if ( category.isNull() )
                category = elm.attribute( QString::fromLatin1( "option-group" ) ); // compatible with KimDaBa 2.0
            QString group = elm.attribute( QString::fromLatin1( "group-name" ) );
            if ( elm.hasAttribute( QString::fromLatin1( "member" ) ) ) {
                QString member = elm.attribute( QString::fromLatin1( "member" ) );
                _members.addMemberToGroup( category, group, member );
            }
            else {
                QStringList members = QStringList::split( QString::fromLatin1( "," ), elm.attribute( QString::fromLatin1( "members" ) ) );
                for( QStringList::Iterator membersIt = members.begin(); membersIt != members.end(); ++membersIt ) {
                    CategoryPtr catPtr = _categoryCollection.categoryForName( category );
                    XMLCategory* cat = static_cast<XMLCategory*>( catPtr.data() );
                    QString member = cat->nameForId( (*membersIt).toInt() );
                    _members.addMemberToGroup( category, group, member );
                }
            }
        }
    }
}

void XMLDB::XMLDB::saveImages( QDomDocument doc, QDomElement top )
{
    ImageInfoList list = _images;

    // Copy files from clipboard to end of overview, so we don't loose them
    for( ImageInfoListConstIterator it = _clipboard.constBegin(); it != _clipboard.constEnd(); ++it ) {
        list.append( *it );
    }

    QDomElement images = doc.createElement( QString::fromLatin1( "images" ) );
    top.appendChild( images );

    for( ImageInfoListIterator it = list.begin(); it != list.end(); ++it ) {
        images.appendChild( (*it)->save( doc ) );
    }
}

void XMLDB::XMLDB::saveBlockList( QDomDocument doc, QDomElement top )
{
    QDomElement blockList = doc.createElement( QString::fromLatin1( "blocklist" ) );
    bool any=false;
    for( QStringList::Iterator it = _blockList.begin(); it != _blockList.end(); ++it ) {
        any=true;
        QDomElement elm = doc.createElement( QString::fromLatin1( "block" ) );
        elm.setAttribute( QString::fromLatin1( "file" ), *it );
        blockList.appendChild( elm );
    }

    if (any)
        top.appendChild( blockList );
}

void XMLDB::XMLDB::saveMemberGroups( QDomDocument doc, QDomElement top )
{
    if ( _members.isEmpty() )
        return;

    QDomElement memberNode = doc.createElement( QString::fromLatin1( "member-groups" ) );
    for( QMapIterator< QString,QMap<QString,QStringList> > it1= _members._members.begin(); it1 != _members._members.end(); ++it1 ) {
        QMap<QString,QStringList> map = it1.data();
        for( QMapIterator<QString,QStringList> it2= map.begin(); it2 != map.end(); ++it2 ) {
            QStringList list = it2.data();
            if ( Options::instance()->useCompressedIndexXML() && !KCmdLineArgs::parsedArgs()->isSet( "export-in-2.1-format" )) {
                QDomElement elm = doc.createElement( QString::fromLatin1( "member" ) );
                elm.setAttribute( QString::fromLatin1( "category" ), it1.key() );
                elm.setAttribute( QString::fromLatin1( "group-name" ), it2.key() );
                QStringList idList;
                for( QStringList::Iterator listIt = list.begin(); listIt != list.end(); ++listIt ) {
                    CategoryPtr catPtr = _categoryCollection.categoryForName( it1.key() );
                    XMLCategory* category = static_cast<XMLCategory*>( catPtr.data() );
                    idList.append( QString::number( category->idForName( *listIt ) ) );
                }
                elm.setAttribute( QString::fromLatin1( "members" ), idList.join( QString::fromLatin1( "," ) ) );
                memberNode.appendChild( elm );
            }
            else {
                for( QStringList::Iterator it3 = list.begin(); it3 != list.end(); ++it3 ) {
                    QDomElement elm = doc.createElement( QString::fromLatin1( "member" ) );
                    memberNode.appendChild( elm );
                    elm.setAttribute( QString::fromLatin1( "category" ), it1.key() );
                    elm.setAttribute( QString::fromLatin1( "group-name" ), it2.key() );
                    elm.setAttribute( QString::fromLatin1( "member" ), *it3 );
                }
            }
        }
    }

    top.appendChild( memberNode );
}

void XMLDB::XMLDB::saveCategories( QDomDocument doc, QDomElement top )
{
    QStringList grps = ImageDB::instance()->categoryCollection()->categoryNames();
    QDomElement options = doc.createElement( QString::fromLatin1("options") );
    top.appendChild( options );


    for( QStringList::Iterator it = grps.begin(); it != grps.end(); ++it ) {
        QDomElement opt = doc.createElement( QString::fromLatin1("option") );
        QString name = *it;
        opt.setAttribute( QString::fromLatin1("name"),  name );
        CategoryPtr category = ImageDB::instance()->categoryCollection()->categoryForName( name );

        opt.setAttribute( QString::fromLatin1( "icon" ), category->iconName() );
        opt.setAttribute( QString::fromLatin1( "show" ), category->doShow() );
        opt.setAttribute( QString::fromLatin1( "viewsize" ), category->viewSize() );
        opt.setAttribute( QString::fromLatin1( "viewtype" ), category->viewType() );

        if ( category->isSpecialCategory() )
            continue;

        QStringList list = category->items();
        for( QStringList::Iterator it2 = list.begin(); it2 != list.end(); ++it2 ) {
            QDomElement val = doc.createElement( QString::fromLatin1("value") );
            val.setAttribute( QString::fromLatin1("value"), *it2 );
            val.setAttribute( QString::fromLatin1( "id" ), static_cast<XMLCategory*>( category.data() )->idForName( *it2 ) );
            opt.appendChild( val );
        }
        options.appendChild( opt );
    }
}

QStringList XMLDB::XMLDB::images()
{
    QStringList result;
    for( ImageInfoListIterator it = _images.begin(); it != _images.end(); ++it ) {
        result.append( (*it)->fileName() );
    }
    return result;
}

QStringList XMLDB::XMLDB::search( const ImageSearchInfo& info, bool requireOnDisk ) const
{
    QStringList result;
    for( ImageInfoListConstIterator it = _images.constBegin(); it != _images.constEnd(); ++it ) {
        bool match = !(*it)->isLocked() && info.match( *it ) && rangeInclude( *it );
        match &= !requireOnDisk || (*it)->imageOnDisk();

        if (match)
            result.append((*it)->fileName());
    }
    return result;
}

void XMLDB::XMLDB::sortAndMergeBackIn( const QStringList& fileList )
{
    ImageInfoList list;

    for( QStringList::ConstIterator it = fileList.begin(); it != fileList.end(); ++it ) {
        list.append( ImageDB::instance()->info( *it ) );
    }
    _images.sortAndMergeBackIn( list );
}

CategoryCollection* XMLDB::XMLDB::categoryCollection()
{
    return &_categoryCollection;
}

KSharedPtr<ImageDateRangeCollection> XMLDB::XMLDB::rangeCollection()
{
    return new XMLImageDateRangeCollection( search( Browser::instance()->currentContext(), false ) );
}

void XMLDB::XMLDB::reorder( const QString& item, const QStringList& selection, bool after )
{
    ImageInfoList list = takeImagesFromSelection( selection );
    insertList( item, list, after );
}

// The selection is know to be sorted wrt the order in the image list.
ImageInfoList XMLDB::XMLDB::takeImagesFromSelection( const QStringList& selection )
{
    QStringList cutList = selection;
    ImageInfoList result;
    for( ImageInfoListIterator it = _images.begin(); it != _images.end() && !cutList.isEmpty(); ) {
        if ( (*it)->fileName() == cutList[0] ) {
            result << *it;
            it = _images.erase(it);
            cutList.pop_front();
        }
        else
            ++it;
    }
    Q_ASSERT( cutList.isEmpty() );

    return result;
}

QStringList XMLDB::XMLDB::insertList( const QString& fileName, const ImageInfoList& list, bool after )
{
    QStringList result;

    ImageInfoListIterator imageIt = _images.begin();
    for( ; imageIt != _images.end(); ++imageIt ) {
        if ( (*imageIt)->fileName() == fileName ) {
            break;
        }
    }

    if ( after )
        imageIt++;
    for( ImageInfoListConstIterator it = list.begin(); it != list.end(); ++it ) {
        _images.insert( imageIt, *it );
        result << (*it)->fileName();
    }
    emit dirty();
    return result;
}


void XMLDB::XMLDB::cutToClipboard( const QStringList& selection )
{
    _clipboard = takeImagesFromSelection( selection );
}

QStringList XMLDB::XMLDB::pasteFromCliboard( const QString& afterFile )
{
    QStringList result = insertList( afterFile, _clipboard, true );
    _clipboard.clear();
    return result;
}

bool XMLDB::XMLDB::isClipboardEmpty()
{
    return _clipboard.isEmpty();
}

int XMLDB::XMLDB::fileVersion()
{
    return _fileVersion;
}

void XMLDB::XMLDB::checkAndWarnAboutVersionConflict()
{
    if ( _fileVersion == 1 ) {
        KMessageBox::information( 0, i18n( "<p>The index.xml file read was from an older version of KimDaBa. "
                                           "KimDaBa read the old format without problems, but to be able to convert back to "
                                           "KimDaBa 2.1 format, you need to run the current KimDaBa using the flag "
                                           "<tt>export-in-2.1-format</tt>, and then save.</p>"),
                              i18n("Old File Format read"), QString::fromLatin1( "version1FileFormatRead" ) );
    }
}

// This function will save an empty config element and a valid configWindowSetup element in the XML file.
// In versions of KimDaBa newer than 2.1, these informations are stored
// using KConfig, rather than in the database, so I need to add them like
// this to make the file readable by KimDaBa 2.1.
void XMLDB::XMLDB::add21CompatXML( QDomElement& top )
{
    QDomDocument doc = top.ownerDocument();
    top.appendChild( doc.createElement( QString::fromLatin1( "config" ) ) );

    QCString conf = QCString( "<configWindowSetup>  <dock>   <name>Label and Dates</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </dock>  <dock>   <name>Image Preview</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </dock>  <dock>   <name>Description</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </dock>  <dock>   <name>Keywords</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </dock>  <dock>   <name>Locations</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </dock>  <dock>   <name>Persons</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </dock>  <splitGroup>   <firstName>Label and Dates</firstName>   <secondName>Description</secondName>   <orientation>0</orientation>   <separatorPos>31</separatorPos>   <name>Label and Dates,Description</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </splitGroup>  <splitGroup>   <firstName>Label and Dates,Description</firstName>   <secondName>Image Preview</secondName>   <orientation>1</orientation>   <separatorPos>70</separatorPos>   <name>Label and Dates,Description,Image Preview</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </splitGroup>  <splitGroup>   <firstName>Locations</firstName>   <secondName>Keywords</secondName>   <orientation>1</orientation>   <separatorPos>50</separatorPos>   <name>Locations,Keywords</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </splitGroup>  <splitGroup>   <firstName>Persons</firstName>   <secondName>Locations,Keywords</secondName>   <orientation>1</orientation>   <separatorPos>34</separatorPos>   <name>Persons,Locations,Keywords</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </splitGroup>  <splitGroup>   <firstName>Label and Dates,Description,Image Preview</firstName>   <secondName>Persons,Locations,Keywords</secondName>   <orientation>0</orientation>   <separatorPos>0</separatorPos>   <name>Label and Dates,Description,Image Preview,Persons,Locations,Keywords</name>   <hasParent>true</hasParent>   <dragEnabled>true</dragEnabled>  </splitGroup>  <centralWidget>Label and Dates,Description,Image Preview,Persons,Locations,Keywords</centralWidget>  <mainDockWidget>Label and Dates</mainDockWidget>  <geometry>   <x>6</x>   <y>6</y>   <width>930</width>   <height>492</height>  </geometry> </configWindowSetup>" );

    QDomDocument tmpDoc;
    tmpDoc.setContent( conf );
    top.appendChild( tmpDoc.documentElement() );
}

ImageInfoPtr XMLDB::XMLDB::createImageInfo( const QString& fileName, const QDomElement& elm, XMLDB* db )
{
    QString label = elm.attribute( QString::fromLatin1("label") );
    QString description = elm.attribute( QString::fromLatin1("description") );

    ImageDate date;
    if ( elm.hasAttribute( QString::fromLatin1( "startDate" ) ) ) {
        QDateTime start;
        QDateTime end;

        QString str = elm.attribute( QString::fromLatin1( "startDate" ) );
        if ( !str.isEmpty() )
            start = QDateTime::fromString( str, Qt::ISODate );

        str = elm.attribute( QString::fromLatin1( "endDate" ) );
        if ( !str.isEmpty() )
            end = QDateTime::fromString( str, Qt::ISODate );
        date = ImageDate( start, end );
    }
    else {
        int yearFrom = 0, monthFrom = 0,  dayFrom = 0, yearTo = 0, monthTo = 0,  dayTo = 0, hourFrom = -1, minuteFrom = -1, secondFrom = -1;

        yearFrom = elm.attribute( QString::fromLatin1("yearFrom"), QString::number( yearFrom) ).toInt();
        monthFrom = elm.attribute( QString::fromLatin1("monthFrom"), QString::number(monthFrom) ).toInt();
        dayFrom = elm.attribute( QString::fromLatin1("dayFrom"), QString::number(dayFrom) ).toInt();
        hourFrom = elm.attribute( QString::fromLatin1("hourFrom"), QString::number(hourFrom) ).toInt();
        minuteFrom = elm.attribute( QString::fromLatin1("minuteFrom"), QString::number(minuteFrom) ).toInt();
        secondFrom = elm.attribute( QString::fromLatin1("secondFrom"), QString::number(secondFrom) ).toInt();

        yearTo = elm.attribute( QString::fromLatin1("yearTo"), QString::number(yearTo) ).toInt();
        monthTo = elm.attribute( QString::fromLatin1("monthTo"), QString::number(monthTo) ).toInt();
        dayTo = elm.attribute( QString::fromLatin1("dayTo"), QString::number(dayTo) ).toInt();
        date = ImageDate( yearFrom, monthFrom, dayFrom, yearTo, monthTo, dayTo, hourFrom, minuteFrom, secondFrom );
    }

    int angle = elm.attribute( QString::fromLatin1("angle"), QString::fromLatin1("0") ).toInt();
    QString md5sum = elm.attribute( QString::fromLatin1( "md5sum" ) );

    _anyImageWithEmptySize |= !elm.hasAttribute( QString::fromLatin1( "width" ) );

    int w = elm.attribute( QString::fromLatin1( "width" ), QString::fromLatin1( "-1" ) ).toInt();
    int h = elm.attribute( QString::fromLatin1( "height" ), QString::fromLatin1( "-1" ) ).toInt();
    QSize size = QSize( w,h );

    ImageInfo* info = new ImageInfo( fileName, label, description, date, angle, md5sum, size );
    ImageInfoPtr result = info;
    for ( QDomNode child = elm.firstChild(); !child.isNull(); child = child.nextSibling() ) {
        if ( child.isElement() ) {
            QDomElement childElm = child.toElement();
            if ( childElm.tagName() == QString::fromLatin1( "options" ) ) {
                readOptions( result, childElm );
            }
            else if ( childElm.tagName() == QString::fromLatin1( "drawings" ) ) {
                result->addDrawing( childElm );
            }
            else {
                KMessageBox::error( 0, i18n("<qt><p>Unknown tag %1, while reading configuration file.</p>"
                                            "<p>Expected one of: Options, Drawings</p></qt>" ).arg( childElm.tagName() ) );
            }
        }
    }

    possibleLoadCompressedCategories( elm, result, db );

    return result;
}

void XMLDB::XMLDB::readOptions( ImageInfoPtr info, QDomElement elm )
{
    Q_ASSERT( elm.tagName() == QString::fromLatin1( "options" ) );

    for ( QDomNode nodeOption = elm.firstChild(); !nodeOption.isNull(); nodeOption = nodeOption.nextSibling() )  {

        if ( nodeOption.isElement() )  {
            QDomElement elmOption = nodeOption.toElement();
            Q_ASSERT( elmOption.tagName() == QString::fromLatin1("option") );
            QString name = elmOption.attribute( QString::fromLatin1("name") );
            if ( name == QString::fromLatin1( "Folder" ) )
                continue; // KimDaBa 2.0 save this to the file, that was a mistake.

            if ( !name.isNull() )  {
                // Read values
                for ( QDomNode nodeValue = elmOption.firstChild(); !nodeValue.isNull();
                      nodeValue = nodeValue.nextSibling() ) {
                    if ( nodeValue.isElement() ) {
                        QDomElement elmValue = nodeValue.toElement();
                        Q_ASSERT( elmValue.tagName() == QString::fromLatin1("value") );
                        QString value = elmValue.attribute( QString::fromLatin1("value") );
                        if ( !value.isNull() )  {
                            info->addOption( name, value );
                        }
                    }
                }
            }
        }
    }
}

void XMLDB::XMLDB::possibleLoadCompressedCategories( const QDomElement& elm, ImageInfoPtr info, XMLDB* db )
{
    if ( db == 0 )
        return;

    QValueList<CategoryPtr> categoryList = db->_categoryCollection.categories();
    for( QValueList<CategoryPtr>::Iterator categoryIt = categoryList.begin(); categoryIt != categoryList.end(); ++categoryIt ) {
        QString categoryName = (*categoryIt)->name();
        QString str = elm.attribute( categoryName );
        if ( !str.isEmpty() ) {
            QStringList list = QStringList::split( QString::fromLatin1( "," ), str);
            for( QStringList::Iterator listIt = list.begin(); listIt != list.end(); ++listIt ) {
                int id = (*listIt).toInt();
                QString name = static_cast<XMLCategory*>((*categoryIt).data())->nameForId(id);
                info->addOption( categoryName, name );
            }
        }
    }
}

