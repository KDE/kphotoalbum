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

#include "xmldb.h"
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
#include "categorycollection.h"
#include "xmldb.moc"
#include <kstandarddirs.h>
#include <qregexp.h>
#include <stdlib.h>

XMLDB::XMLDB( const QString& configFile )
{
    Util::checkForBackupFile( configFile );
    QDomElement top = readConfigFile( configFile );
    QDomElement categories;
    QDomElement images;
    QDomElement blockList;
    QDomElement memberGroups;
    readTopNodeInConfigDocument( configFile, top, &categories, &images, &blockList, &memberGroups );

    loadCategories( categories );
    loadImages( images );
    loadBlockList( blockList );
    loadMemberGroups( memberGroups );

    // PENDING(blackie) Where should these go?
    checkIfImagesAreSorted();
    checkIfAllImagesHasSizeAttributes();
}

int XMLDB::totalCount() const
{
    return _images.count();
}

ImageInfoList XMLDB::search( const ImageSearchInfo& info, bool requireOnDisk ) const
{
    ImageInfoList result;
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        bool match = !(*it)->isLocked() && info.match( *it ) && rangeInclude( *it );
        match &= !requireOnDisk || (*it)->imageOnDisk();

        if (match)
            result.append(*it);
    }
    return result;
}


int XMLDB::count( const ImageSearchInfo& info )
{
    int count = search( info ).count();
    return count;
}

QMap<QString,int> XMLDB::classify( const ImageSearchInfo& info, const QString &group )
{
    QMap<QString, int> map;
    GroupCounter counter( group );
    QDict<void> alreadyMatched = findAlreadyMatched( info, group );

    ImageSearchInfo noMatchInfo = info;
    QString currentMatchTxt = noMatchInfo.option( group );
    if ( currentMatchTxt.isEmpty() )
        noMatchInfo.setOption( group, ImageDB::NONE() );
    else
        noMatchInfo.setOption( group, QString::fromLatin1( "%1 & %2" ).arg(currentMatchTxt).arg(ImageDB::NONE()) );

    // Iterate through the whole database of images.
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        bool match = !(*it)->isLocked() && info.match( *it ) && rangeInclude( *it );
        if ( match ) { // If the given image is currently matched.

            // Now iterate through all the categories the current image
            // contains, and increase them in the map mapping from option
            // group to count for this group.
            QStringList list = (*it)->optionValue(group);
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

void XMLDB::renameOptionGroup( const QString& oldName, const QString newName )
{
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        (*it)->renameOptionGroup( oldName, newName );
    }
}

void XMLDB::addToBlockList( const ImageInfoList& list )
{
    for( ImageInfoListIterator it( list ); *it; ++it) {
        _blockList << (*it)->fileName( true );
        _images.removeRef( *it );
    }
    emit totalChanged( _images.count() );
}

void XMLDB::deleteList( const ImageInfoList& list )
{
    for( ImageInfoListIterator it( list ); *it; ++it ) {
        _images.removeRef( *it );
    }
    emit totalChanged( _images.count() );
}

void XMLDB::renameOption( Category* category, const QString& oldName, const QString& newName )
{
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        (*it)->renameOption( category->name(), oldName, newName );
    }
}

void XMLDB::deleteOption( Category* category, const QString& option )
{
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        (*it)->removeOption( category->name(), option );
    }
}

void XMLDB::lockDB( bool lock, bool exclude  )
{
    ImageSearchInfo info = Options::instance()->currentLock();
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
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


void XMLDB::slotReread(ImageInfoList rereadList, int mode)
{
    // Do here a reread of the exif info and change the info correctly in the database without loss of previous added data
    QProgressDialog  dialog( i18n("<qt><p><b>Loading time information from images</b></p>"
                                  "<p>Depending on the number of images, this may take some time.</p></qt>"),
                             i18n("Cancel"), rereadList.count() );

    int count=0;
    for( ImageInfoListIterator it( rereadList ); *it; ++it, ++count ) {
        if ( count % 10 == 0 ) {
            dialog.setProgress( count ); // ensure to call setProgress(0)
            qApp->eventLoop()->processEvents( QEventLoop::AllEvents );

            if ( dialog.wasCanceled() )
                return;
        }

        QFileInfo fi( (*it)->fileName() );

        if (fi.exists())
            (*it)->readExif((*it)->fileName(), mode);
        emit dirty();
    }
}


void XMLDB::addImage( ImageInfo* info )
{
    _images.append( info );
    emit totalChanged( _images.count() );
    emit dirty();
}

ImageInfo* XMLDB::find( const QString& fileName ) const
{
    static QMap<QString, ImageInfo* > fileMap;

    if ( fileMap.contains( fileName ) )
        return fileMap[ fileName ];
    else {
        fileMap.clear();
        for( ImageInfoListIterator it( _images ); *it; ++it ) {
            fileMap.insert( (*it)->fileName(), *it );
        }
        if ( fileMap.contains( fileName ) )
            return fileMap[ fileName ];
    }
    return 0;
}

QDict<void> XMLDB::findAlreadyMatched( const ImageSearchInfo& info, const QString &group )
{
    QDict<void> map;
    QString str = info.option( group );
    if ( str.contains( QString::fromLatin1( "|" ) ) ) {
        return map;
    }

    QStringList list = QStringList::split( QString::fromLatin1( "&" ), str );
    for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        QString nm = (*it).stripWhiteSpace();
        if (! nm.contains( QString::fromLatin1( "!" ) ) )
            map.insert( nm, (void*) 0x1 /* something different from 0x0 */ );
    }
    return map;
}

void XMLDB::checkIfImagesAreSorted()
{
    if ( !KMessageBox::shouldBeShownContinue( QString::fromLatin1( "checkWhetherImagesAreSorted" ) ) )
        return;

    QDateTime last( QDate( 1900, 1, 1 ) );
    bool wrongOrder = false;
    for( ImageInfoListIterator it( _images ); !wrongOrder && *it; ++it ) {
        if ( last > (*it)->startDate().min() )
            wrongOrder = true;
        last = (*it)->startDate().min();
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

bool XMLDB::rangeInclude( ImageInfo* info ) const
{
    if (_selectionRange.start().isNull() )
        return true;

    ImageDateRange::MatchType tp = info->dateRange().isIncludedIn( _selectionRange );
    if ( _includeFuzzyCounts )
        return ( tp == ImageDateRange::ExactMatch || tp == ImageDateRange::RangeMatch );
    else
        return ( tp == ImageDateRange::ExactMatch );
}

void XMLDB::checkIfAllImagesHasSizeAttributes()
{
    QTime time;
    time.start();
    if ( !KMessageBox::shouldBeShownContinue( QString::fromLatin1( "checkWhetherAllImagesIncludesSize" ) ) )
        return;

    if ( ImageInfo::_anyImageWithEmptySize ) {
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


const MemberMap& XMLDB::memberMap()
{
    return _members;
}

void XMLDB::setMemberMap( const MemberMap& members )
{
    _members = members;
}

void XMLDB::loadCategories( const QDomElement& elm )
{
    Q_ASSERT( elm.tagName() == QString::fromLatin1( "options" ) );
    CategoryCollection* categories = CategoryCollection::instance();

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

                Category* cat = categories->categoryForName( name ); // Special Categories are already created.
                if ( !cat ) {
                    cat = new Category( name, icon, size, type, show );
                    categories->addCategory( cat );
                }

                // Read values
                QStringList items;
                for ( QDomNode nodeValue = elmOption.firstChild(); !nodeValue.isNull();
                      nodeValue = nodeValue.nextSibling() ) {
                    if ( nodeValue.isElement() ) {
                        QDomElement elmValue = nodeValue.toElement();
                        Q_ASSERT( elmValue.tagName() == QString::fromLatin1("value") );
                        QString value = elmValue.attribute( QString::fromLatin1("value") );
                        items.append( value );
                    }
                }
                cat->setItems( items );
            }
        }
    }
}

void XMLDB::save( const QString& fileName )
{
    QDomDocument doc;

    doc.appendChild( doc.createProcessingInstruction( QString::fromLatin1("xml"), QString::fromLatin1("version=\"1.0\" encoding=\"UTF-8\"") ) );
    QDomElement top = doc.createElement( QString::fromLatin1("KimDaBa") );
    doc.appendChild( top );

    saveImages( doc, top );
    saveBlockList( doc, top );
    saveMemberGroups( doc, top );
    saveCategories( doc, top );

    QFile out( fileName );

    if ( !out.open( IO_WriteOnly ) )
        KMessageBox::sorry( MainView::theMainView(), i18n( "Could not open file '%1'." ).arg( fileName ) );
    else {
        QCString s = doc.toCString();
        out.writeBlock( s.data(), s.size()-1 );
        out.close();
    }
}


MD5Map* XMLDB::md5Map()
{
    return &_md5map;
}

bool XMLDB::isBlocking( const QString& fileName )
{
    return _blockList.contains( fileName );
}

QDomElement XMLDB::readConfigFile( const QString& configFile )
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

void XMLDB::readTopNodeInConfigDocument( const QString& configFile, QDomElement top, QDomElement* options, QDomElement* images,
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

void XMLDB::loadImages( const QDomElement& images )
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
            ImageInfo* info = load( fileName, elm );
            _images.append(info);
            _md5map.insert( info->MD5Sum(), fileName );
        }
    }

}

ImageInfo* XMLDB::load( const QString& fileName, QDomElement elm )
{
    ImageInfo* info = new ImageInfo( fileName, elm );
    return info;
}

void XMLDB::loadBlockList( const QDomElement& blockList )
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

void XMLDB::loadMemberGroups( const QDomElement& memberGroups )
{
    for ( QDomNode node = memberGroups.firstChild(); !node.isNull(); node = node.nextSibling() ) {
        if ( node.isElement() ) {
            QDomElement elm = node.toElement();
            QString category = elm.attribute( QString::fromLatin1( "category" ) );
            if ( category.isNull() )
                category = elm.attribute( QString::fromLatin1( "option-group" ) ); // compatible with KimDaBa 2.0
            QString group = elm.attribute( QString::fromLatin1( "group-name" ) );
            QString member = elm.attribute( QString::fromLatin1( "member" ) );
            _members.addMemberToGroup( category, group, member );
        }
    }
}

void XMLDB::saveImages( QDomDocument doc, QDomElement top )
{
    ImageInfoList list = _images;

    // Copy files from clipboard to end of overview, so we don't loose them
    for( ImageInfoListIterator it(_clipboard); *it; ++it ) {
        list.append( *it );
    }

    QDomElement images = doc.createElement( QString::fromLatin1( "images" ) );
    top.appendChild( images );

    for( ImageInfoListIterator it( list ); *it; ++it ) {
        images.appendChild( (*it)->save( doc ) );
    }
}

void XMLDB::saveBlockList( QDomDocument doc, QDomElement top )
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

void XMLDB::saveMemberGroups( QDomDocument doc, QDomElement top )
{
    if ( _members.isEmpty() )
        return;

    QDomElement memberNode = doc.createElement( QString::fromLatin1( "member-groups" ) );
    for( QMapIterator< QString,QMap<QString,QStringList> > it1= _members._members.begin(); it1 != _members._members.end(); ++it1 ) {
        QMap<QString,QStringList> map = it1.data();
        for( QMapIterator<QString,QStringList> it2= map.begin(); it2 != map.end(); ++it2 ) {
            QStringList list = it2.data();
            for( QStringList::Iterator it3 = list.begin(); it3 != list.end(); ++it3 ) {
                QDomElement elm = doc.createElement( QString::fromLatin1( "member" ) );
                memberNode.appendChild( elm );
                elm.setAttribute( QString::fromLatin1( "category" ), it1.key() );
                elm.setAttribute( QString::fromLatin1( "group-name" ), it2.key() );
                elm.setAttribute( QString::fromLatin1( "member" ), *it3 );
            }
        }
    }

    top.appendChild( memberNode );
}

void XMLDB::saveCategories( QDomDocument doc, QDomElement top )
{
    QStringList grps = CategoryCollection::instance()->categoryNames();
    QDomElement options = doc.createElement( QString::fromLatin1("options") );
    top.appendChild( options );


    for( QStringList::Iterator it = grps.begin(); it != grps.end(); ++it ) {
        QDomElement opt = doc.createElement( QString::fromLatin1("option") );
        QString name = *it;
        opt.setAttribute( QString::fromLatin1("name"),  name );
        Category* category = CategoryCollection::instance()->categoryForName( name );

        opt.setAttribute( QString::fromLatin1( "icon" ), category->iconName() );
        opt.setAttribute( QString::fromLatin1( "show" ), category->doShow() );
        opt.setAttribute( QString::fromLatin1( "viewsize" ), category->viewSize() );
        opt.setAttribute( QString::fromLatin1( "viewtype" ), category->viewType() );

        // we don t save the values for the option "Folder" since it is automatically set
        // but we keep the <option> element to allow to save user's preference for viewsize,icon,show,name
        QStringList list;
        if ( category->isSpecialCategory() )
            list = QStringList();
        else
            list = category->items();
        bool any = false;
        for( QStringList::Iterator it2 = list.begin(); it2 != list.end(); ++it2 ) {
            QDomElement val = doc.createElement( QString::fromLatin1("value") );
            val.setAttribute( QString::fromLatin1("value"), *it2 );
            opt.appendChild( val );
            any = true;
        }
        options.appendChild( opt );
    }
}

