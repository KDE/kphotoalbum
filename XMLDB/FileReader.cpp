#include "FileReader.h"
#include "Database.h"
#include "XMLCategory.h"
#include "DB/MD5Map.h"
#include <kmessagebox.h>
#include "MainWindow/Window.h"
#include <klocale.h>
#include <qfile.h>
#include <kstandarddirs.h>
#include <qregexp.h>
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

void XMLDB::FileReader::read( const QString& configFile )
{
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


void XMLDB::FileReader::readTopNodeInConfigDocument( const QString& configFile, QDomElement top, QDomElement* options, QDomElement* images,
QDomElement* blockList, QDomElement* memberGroups )
{
    for ( QDomNode node = top.firstChild(); !node.isNull(); node = node.nextSibling() ) {
        if ( node.isElement() ) {
            QDomElement elm = node.toElement();
            QString tag = elm.tagName().lower();
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
                KMessageBox::error( MainWindow::Window::theMainWindow(),
                                    i18n("Error in file %1: unexpected element: '%2*").arg( configFile ).arg( tag ) );
            }
        }
    }

    if ( options->isNull() )
        KMessageBox::sorry( MainWindow::Window::theMainWindow(), i18n("Unable to find 'Options' tag in configuration file %1.").arg( configFile ) );
    if ( images->isNull() )
        KMessageBox::sorry( MainWindow::Window::theMainWindow(), i18n("Unable to find 'Images' tag in configuration file %1.").arg( configFile ) );
}

void XMLDB::FileReader::createSpecialCategories()
{
    DB::CategoryPtr folderCat = _db->_categoryCollection.categoryForName( QString::fromLatin1( "Folder" ) );
    if( folderCat == 0 ) {
        folderCat = new XMLCategory( QString::fromLatin1("Folder"), QString::fromLatin1("folder"),
                                     DB::Category::ListView, 32, false );
        _db->_categoryCollection.addCategory( folderCat );
    }
    folderCat->setSpecialCategory( true );
    dynamic_cast<XMLCategory*>( folderCat.data() )->setShouldSave( false );

    DB::CategoryPtr tokenCat = _db->_categoryCollection.categoryForName( QString::fromLatin1( "Tokens" ) );
    if ( !tokenCat ) {
        tokenCat = new XMLCategory( QString::fromLatin1("Tokens"), QString::fromLatin1("cookie"),
                                    DB::Category::ListView, 32, true );
        _db->_categoryCollection.addCategory( tokenCat );
    }
    tokenCat->setSpecialCategory( true );

    // KPhotoAlbum 2.2 did not write the tokens to the category section, so unless we do this small trick they
    // will not show up when importing.
    for ( char ch = 'A'; ch < 'Z'; ++ch )
        tokenCat->addItem( QString::fromLatin1("%1").arg( QChar( ch) ) );

    DB::CategoryPtr mediaCat = _db->_categoryCollection.categoryForName( QString::fromLatin1( "Media Type" ) );
    if ( !mediaCat ) {
        mediaCat = new XMLCategory( QString::fromLatin1("Media Type"), QString::fromLatin1("video"),
                                    DB::Category::ListView, 32, true );
        mediaCat->addItem( QString::fromLatin1( "Image" ) );
        mediaCat->addItem( QString::fromLatin1( "Video" ) );

        _db->_categoryCollection.addCategory( mediaCat );
    }
    mediaCat->setSpecialCategory( true );
    dynamic_cast<XMLCategory*>( mediaCat.data() )->setShouldSave( false );
}

void XMLDB::FileReader::loadCategories( const QDomElement& elm )
{
    // options is for KimDaBa 2.1 compatibility
    Q_ASSERT( elm.tagName().lower() == QString::fromLatin1( "categories" ) || elm.tagName().lower() == QString::fromLatin1( "options" ) );

    for ( QDomNode nodeOption = elm.firstChild(); !nodeOption.isNull(); nodeOption = nodeOption.nextSibling() )  {

        if ( nodeOption.isElement() )  {
            QDomElement elmOption = nodeOption.toElement();
            // option is for KimDaBa 2.1 compatibility
            Q_ASSERT( elmOption.tagName().lower() == QString::fromLatin1("category") ||
                      elmOption.tagName() == QString::fromLatin1("option").lower() );
            QString name = unescape( elmOption.attribute( QString::fromLatin1("name") ) );

            if ( !name.isNull() )  {
                // Read Category info
                QString icon= elmOption.attribute( QString::fromLatin1("icon") );
                DB::Category::ViewType type =
                    (DB::Category::ViewType) elmOption.attribute( QString::fromLatin1("viewtype"), QString::fromLatin1( "0" ) ).toInt();
                bool show = (bool) elmOption.attribute( QString::fromLatin1( "show" ),
                                                        QString::fromLatin1( "1" ) ).toInt();
                int thumbnailSize = elmOption.attribute( QString::fromLatin1( "thumbnailsize" ), QString::fromLatin1( "32" ) ).toInt();

                DB::CategoryPtr cat = _db->_categoryCollection.categoryForName( name );
                Q_ASSERT ( !cat );
                cat = new XMLCategory( name, icon, type, thumbnailSize, show );
                _db->_categoryCollection.addCategory( cat );

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

    createSpecialCategories();
}

void XMLDB::FileReader::loadImages( const QDomElement& images )
{
    QString directory = Settings::SettingsData::instance()->imageDirectory();

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
            DB::ImageInfoPtr info = load( fileName, elm );
            _db->_images.append(info);
            _db->_md5map.insert( info->MD5Sum(), fileName );
        }
    }

}

void XMLDB::FileReader::loadBlockList( const QDomElement& blockList )
{
    for ( QDomNode node = blockList.firstChild(); !node.isNull(); node = node.nextSibling() )  {
        QDomElement elm;
        if ( node.isElement() )
            elm = node.toElement();
        else
            continue;

        QString fileName = elm.attribute( QString::fromLatin1( "file" ) );
        if ( !fileName.isEmpty() )
            _db->_blockList << fileName;
    }
}

void XMLDB::FileReader::loadMemberGroups( const QDomElement& memberGroups )
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
                _db->_members.addMemberToGroup( category, group, member );
            }
            else {
                QStringList members = QStringList::split( QString::fromLatin1( "," ), elm.attribute( QString::fromLatin1( "members" ) ) );
                for( QStringList::Iterator membersIt = members.begin(); membersIt != members.end(); ++membersIt ) {
                    DB::CategoryPtr catPtr = _db->_categoryCollection.categoryForName( category );
                    XMLCategory* cat = static_cast<XMLCategory*>( catPtr.data() );
                    QString member = cat->nameForId( (*membersIt).toInt() );
                    Q_ASSERT( !member.isNull() );
                    _db->_members.addMemberToGroup( category, group, member );
                }
            }
        }
    }
}

void XMLDB::FileReader::checkIfImagesAreSorted()
{
    if ( !KMessageBox::shouldBeShownContinue( QString::fromLatin1( "checkWhetherImagesAreSorted" ) ) )
        return;

    QDateTime last( QDate( 1900, 1, 1 ) );
    bool wrongOrder = false;
    for( DB::ImageInfoListIterator it = _db->_images.begin(); !wrongOrder && it != _db->_images.end(); ++it ) {
        if ( last > (*it)->date().start() )
            wrongOrder = true;
        last = (*it)->date().start();
    }

    if ( wrongOrder ) {
        KMessageBox::information( MainWindow::Window::theMainWindow(),
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

void XMLDB::FileReader::checkIfAllImagesHasSizeAttributes()
{
    QTime time;
    time.start();
    if ( !KMessageBox::shouldBeShownContinue( QString::fromLatin1( "checkWhetherAllImagesIncludesSize" ) ) )
        return;

    if ( _db->_anyImageWithEmptySize ) {
        KMessageBox::information( MainWindow::Window::theMainWindow(),
                                  i18n("<qt><p>Not all the images in the database have information about image sizes; this is needed to "
                                       "get the best result in the thumbnail view. To fix this, simply go to the <tt>Maintainance</tt> menu, and first "
                                       "choose <tt>Remove All Thumbnails</tt>, and after that choose <tt>Build Thumbnails</tt>.</p>"
                                       "<p>Not doing so will result in extra space around images in the thumbnail view - that is all - so "
                                       "there is no urgency in doing it.</p></qt>"),
                                  i18n("Not All Images Have Size Information"),
                                  QString::fromLatin1( "checkWhetherAllImagesIncludesSize" ) );
    }

}

void XMLDB::FileReader::checkAndWarnAboutVersionConflict()
{
    if ( _fileVersion == 1 ) {
        KMessageBox::information( 0, i18n( "<p>The index.xml file read was from an older version of KPhotoAlbum. "
                                           "KPhotoAlbum read the old format without problems, but to be able to convert back to "
                                           "KimDaBa 2.1 format, you need to run the current KPhotoAlbum using the flag "
                                           "<tt>export-in-2.1-format</tt>, and then save.</p>"),
                                  i18n("Old File Format read"), QString::fromLatin1( "version1FileFormatRead" ) );
    }
}

DB::ImageInfoPtr XMLDB::FileReader::load( const QString& fileName, QDomElement elm )
{
    DB::ImageInfoPtr info = XMLDB::Database::createImageInfo( fileName, elm, _db );
    info->createFolderCategoryItem( _db->_categoryCollection.categoryForName(QString::fromLatin1("Folder")), _db->_members );
    return info;
}

QDomElement XMLDB::FileReader::readConfigFile( const QString& configFile )
{
    QDomDocument doc;
    QFile file( configFile );
    if ( !file.exists() ) {
        // Load a default setup
        QFile file( locate( "data", QString::fromLatin1( "kphotoalbum/default-setup" ) ) );
        if ( !file.open( IO_ReadOnly ) ) {
            KMessageBox::information( 0, i18n( "<qt><p>KPhotoAlbum was unable to load a default setup, which indicates an installation error</p>"
                                               "<p>If you have installed KPhotoAlbum yourself, then you must remember to set the environment variable "
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
            KMessageBox::error( MainWindow::Window::theMainWindow(), i18n("Unable to open '%1' for reading").arg( configFile ), i18n("Error Running Demo") );
            exit(-1);
        }

        QString errMsg;
        int errLine;
        int errCol;

        if ( !doc.setContent( &file, false, &errMsg, &errLine, &errCol )) {
            KMessageBox::error( MainWindow::Window::theMainWindow(), i18n("Error on line %1 column %2 in file %3: %4").arg( errLine ).arg( errCol ).arg( configFile ).arg( errMsg ) );
            exit(-1);
        }
    }

    // Now read the content of the file.
    QDomElement top = doc.documentElement();
    if ( top.isNull() ) {
        KMessageBox::error( MainWindow::Window::theMainWindow(), i18n("Error in file %1: No elements found").arg( configFile ) );
        exit(-1);
    }

    if ( top.tagName().lower() != QString::fromLatin1( "kphotoalbum" ) &&
         top.tagName().lower() != QString::fromLatin1( "kimdaba" ) ) { // KimDaBa compatibility
        KMessageBox::error( MainWindow::Window::theMainWindow(), i18n("Error in file %1: expected 'KPhotoAlbum' as top element but found '%2'").arg( configFile ).arg( top.tagName() ) );
        exit(-1);
    }

    file.close();
    return top;
}

QString XMLDB::FileReader::unescape( const QString& str )
{
    QString tmp( str );
    tmp.replace( QString::fromLatin1( "_" ), QString::fromLatin1( " " ) );
    return tmp;
}


