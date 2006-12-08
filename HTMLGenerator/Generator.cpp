/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "Generator.h"
#include "kdeversion.h"
#include <klocale.h>
#include <qfile.h>
#include <qapplication.h>
#include <qeventloop.h>
#include "ImageManager/Manager.h"
#include <kfiledialog.h>
#include <kstandarddirs.h>
#include <krun.h>
#include <kio/job.h>
#include <kmessagebox.h>
#include "Utilities/Util.h"
#include <kdebug.h>
#include <qdir.h>
#include "ImportExport/Export.h"
#include "DB/CategoryCollection.h"
#include "DB/ImageInfo.h"
#include "DB/ImageDB.h"
#ifdef HASEXIV2
#  include "Exif/Info.h"
#endif
#include <ktempdir.h>
#include "ImageSizeCheckBox.h"
#include "Setup.h"

HTMLGenerator::Generator::Generator( const Setup& setup, QWidget* parent )
    : QProgressDialog( i18n("Generating images for HTML page "), i18n("&Cancel"), 0, parent ), _hasEnteredLoop( false )
{
    _setup = setup;
}

void HTMLGenerator::Generator::generate()
{
    _tempDir = KTempDir().name();

    // Generate .kim file
    if ( _setup.generateKimFile() ) {
        bool ok;
        QString destURL = _setup.destURL();
        if ( destURL.isEmpty() )
            destURL = _setup.baseURL();

        ImportExport::Export exp( _setup.imageList(), kimFileName( false ), false, -1, ImportExport::ManualCopy, destURL, ok, true );
        if ( !ok )
            return;
    }

    // prepare the progress dialog
    _total = _waitCounter = calculateSteps();
    setTotalSteps( _total );
    setProgress( 0 );
    connect( this, SIGNAL( cancelled() ), this, SLOT( slotCancelGenerate() ) );

    _nameMap = Utilities::createUniqNameMap( _setup.imageList(), false, QString::null );

    // Itertate over each of the image sizes needed.
    for( QValueList<ImageSizeCheckBox*>::ConstIterator sizeIt = _setup.activeResolutions().begin();
         sizeIt != _setup.activeResolutions().end(); ++sizeIt ) {

        bool ok = generateIndexPage( (*sizeIt)->width(), (*sizeIt)->height() );
        if ( !ok )
            return;
        for ( uint index = 0; index < _setup.imageList().count(); ++index ) {
            QString current = _setup.imageList()[index];
            QString prev;
            QString next;
            if ( index != 0 )
                prev = _setup.imageList()[index-1];
            if ( index != _setup.imageList().count() -1 )
                next = _setup.imageList()[index+1];
            ok = generateContentPage( (*sizeIt)->width(), (*sizeIt)->height(), prev, current, next );
            if (!ok)
                return;
        }
    }

    // Now generate the thumbnail images
    for( QStringList::ConstIterator it = _setup.imageList().begin(); it != _setup.imageList().end(); ++it ) {
        if ( wasCanceled() )
            return;

        createImage( *it, _setup.thumbSize() );
    }

    if ( wasCanceled() )
        return;

    if ( _waitCounter > 0 ) {
        _hasEnteredLoop = true;
        qApp->eventLoop()->enterLoop();
    }

    if ( wasCanceled() )
        return;

    bool ok = linkIndexFile();
    if ( !ok )
        return;

    // Copy over the mainpage.css, imagepage.css
    QString themeDir, themeAuthor, themeName;
    getThemeInfo( &themeDir, &themeName, &themeAuthor );
    QDir dir( themeDir );
    QStringList files = dir.entryList( QDir::Files );
    if( files.count() < 1 )
        kdDebug() << QString::fromLatin1("theme '%1' doesn't have enough files to be a theme").arg( themeDir ) << endl;

    for( QStringList::Iterator it = files.begin(); it != files.end(); ++it ) {
        if( *it == QString::fromLatin1("kphotoalbum.theme") ||
            *it == QString::fromLatin1("mainpage.html") ||
            *it == QString::fromLatin1("imagepage.html")) continue;
        QString from = QString::fromLatin1("%1%2").arg( themeDir ).arg(*it);
        QString to = _tempDir+QString::fromLatin1("/") + *it;
        ok = Utilities::copy( from, to );
        if ( !ok ) {
            KMessageBox::error( this, i18n("Error copying %1 to %2").arg( from ).arg( to ) );
            return;
        }
    }


    // Copy files over to destination.
    QString outputDir = _setup.baseDir() + QString::fromLatin1( "/" ) + _setup.outputDir();
    KIO::CopyJob* job = KIO::move( KURL(_tempDir), KURL(outputDir) );
    connect( job, SIGNAL( result( KIO::Job* ) ), this, SLOT( showBrowser() ) );

    qApp->eventLoop()->enterLoop();
    return;
}

bool HTMLGenerator::Generator::generateIndexPage( int width, int height )
{
    QString themeDir, themeAuthor, themeName;
    getThemeInfo( &themeDir, &themeName, &themeAuthor );
    QString content = Utilities::readFile( QString::fromLatin1( "%1mainpage.html" ).arg( themeDir ) );
    if ( content.isNull() )
        return false;

    content = QString::fromLatin1("<!--\nMade with KPhotoAlbum. (http://www.kphotoalbum.org/)\nCopyright &copy; Jesper K. Pedersen\nTheme %1 by %2\n-->\n").arg( themeName ).arg( themeAuthor ) + content;

    content.replace( QString::fromLatin1( "**DESCRIPTION**" ), _setup.description() );
    content.replace( QString::fromLatin1( "**TITLE**" ), _setup.title() );
    QString kimLink = QString::fromLatin1( "Share and Enjoy <a href=\"%1\">KPhotoAlbum export file</a>" ).arg( kimFileName( true ) );
    if ( _setup.generateKimFile() )
        content.replace( QString::fromLatin1( "**KIMFILE**" ), kimLink );
    else
        content.replace( QString::fromLatin1( "**KIMFILE**" ), QString::null );
    QDomDocument doc;

    QDomElement elm;
    QDomElement col;

    // -------------------------------------------------- Thumbnails
    // Initially all of the HTML generation was done using QDom, but it turned out in the end
    // to be much less code simply concatenating strings. This part, however, is easier using QDom
    // so we keep it using QDom.
    int count = 0;
    int cols = _setup.numOfCols();
    QDomElement row;
    for( QStringList::ConstIterator it = _setup.imageList().begin(); it != _setup.imageList().end(); ++it ) {
        if ( wasCanceled() )
            return false;

        if ( count % cols == 0 ) {
            row = doc.createElement( QString::fromLatin1( "tr" ) );
            row.setAttribute( QString::fromLatin1( "class" ), QString::fromLatin1( "thumbnail-row" ) );
            doc.appendChild( row );
            count = 0;
        }

        col = doc.createElement( QString::fromLatin1( "td" ) );
        col.setAttribute( QString::fromLatin1( "class" ), QString::fromLatin1( "thumbnail-col" ) );
        row.appendChild( col );

        QDomElement href = doc.createElement( QString::fromLatin1( "a" ) );
        href.setAttribute( QString::fromLatin1( "href" ),
                           namePage( width, height, DB::ImageDB::instance()->info(*it)->fileName(false) ) ); // PENDING(blackie) cleanup
        col.appendChild( href );

        QDomElement img = doc.createElement( QString::fromLatin1( "img" ) );
        img.setAttribute( QString::fromLatin1( "src" ),
                          nameImage( *it, _setup.thumbSize() ) );
        img.setAttribute( QString::fromLatin1( "alt" ),
                          nameImage( *it, _setup.thumbSize() ) );
        href.appendChild( img );
        ++count;
    }

    content.replace( QString::fromLatin1( "**THUMBNAIL-TABLE**" ), doc.toString() );

    // -------------------------------------------------- Resolutions
    QString resolutions;
    QValueList<ImageSizeCheckBox*> actRes = _setup.activeResolutions();
    qHeapSort(actRes);

    if ( actRes.count() > 1 ) {
        resolutions += QString::fromLatin1( "Resolutions: " );
        for( QValueList<ImageSizeCheckBox*>::ConstIterator sizeIt = actRes.begin();
             sizeIt != actRes.end(); ++sizeIt ) {

            int w = (*sizeIt)->width();
            int h = (*sizeIt)->height();
            QString page = QString::fromLatin1( "index-%1.html" ).arg( ImageSizeCheckBox::text( w, h, true ) );
            QString text = (*sizeIt)->text(false);

            resolutions += QString::fromLatin1( " " );
            if ( width == w && height == h ) {
                resolutions += text;
            }
            else {
                resolutions += QString::fromLatin1( "<a href=\"%1\">%2</a>" ).arg( page ).arg( text );
            }
        }
    }

    content.replace( QString::fromLatin1( "**RESOLUTIONS**" ), resolutions );

    if ( wasCanceled() )
        return false;

    // -------------------------------------------------- write to file
    QString fileName = _tempDir + QString::fromLatin1("/index-%1.html" )
                       .arg(ImageSizeCheckBox::text(width,height,true));
    bool ok = writeToFile( fileName, content );

    if ( !ok )
        return false;

    return true;
}

bool HTMLGenerator::Generator::generateContentPage( int width, int height, const QString& prev, const QString& current,
                                                    const QString& next )
{
    QString themeDir, themeAuthor, themeName;
    getThemeInfo( &themeDir, &themeName, &themeAuthor );
    QString content = Utilities::readFile( QString::fromLatin1( "%1imagepage.html" ).arg( themeDir ));
    if ( content.isNull() )
        return false;

    DB::ImageInfoPtr info = DB::ImageDB::instance()->info( current );

    content = QString::fromLatin1("<!--\nMade with KPhotoAlbum. (http://www.kphotoalbum.org/)\nCopyright &copy; Jesper K. Pedersen\nTheme %1 by %2\n-->\n").arg( themeName ).arg( themeAuthor ) + content;

    content.replace( QString::fromLatin1( "**TITLE**" ), info->label() );


    // Image or video content
    if ( Utilities::isVideo( current ) ) {
        QString videoFile = createVideo( current );
        if ( _setup.inlineMovies() )
            content.replace( QString::fromLatin1( "**IMAGE_OR_VIDEO**" ),
                             QString::fromLatin1( "<object data=\"%1\"><img src=\"%2\"/></object>"
                                                  "<a href=\"%3\"><img src=\"download.png\"/></a>")
                             .arg(videoFile).arg( createImage( current, 256 ) ).arg( videoFile ) );
        else
            content.replace( QString::fromLatin1( "**IMAGE_OR_VIDEO**" ),
                             QString::fromLatin1( "<a href=\"**NEXTPAGE**\"><img src=\"%2\"/></a>"
                                                  "<a href=\"%1\"><img src=\"download.png\"/></a>")
                             .arg(videoFile).arg( createImage( current, 256 ) ) );
    }

    else
        content.replace( QString::fromLatin1( "**IMAGE_OR_VIDEO**" ),
                         QString::fromLatin1( "<a href=\"**NEXTPAGE**\"><img src=\"%1\" alt=\"%1\"/></a>")
                         .arg(createImage( current, width ) ) );


    // -------------------------------------------------- Links
    QString link;

    // prev link
    if ( !prev.isNull() )
        link = QString::fromLatin1( "<a href=\"%1\">prev</a>" ).arg( namePage( width, height, prev ) );
    else
        link = QString::fromLatin1( "prev" );
    content.replace( QString::fromLatin1( "**PREV**" ), link );


    // index link
    link = QString::fromLatin1( "<a href=\"index-%1.html\">index</a>" ).arg(ImageSizeCheckBox::text(width,height,true));
    content.replace( QString::fromLatin1( "**INDEX**" ), link );

    // Next Link
    if ( !next.isNull() )
        link = QString::fromLatin1( "<a href=\"%1\">next</a>" ).arg( namePage( width, height, next ) );
    else
        link = QString::fromLatin1( "next" );
    content.replace( QString::fromLatin1( "**NEXT**" ), link );

    if ( !next.isNull() )
        link = namePage( width, height, next );
    else
        link = QString::fromLatin1( "index-%1.html" ).arg(ImageSizeCheckBox::text(width,height,true));

    content.replace( QString::fromLatin1( "**NEXTPAGE**" ), link );


    // -------------------------------------------------- Resolutions
    QString resolutions;
    const QValueList<ImageSizeCheckBox*>& actRes = _setup.activeResolutions();
    if ( actRes.count() > 1 ) {
        for( QValueList<ImageSizeCheckBox*>::ConstIterator sizeIt = actRes.begin();
             sizeIt != actRes.end(); ++sizeIt ) {
            int w = (*sizeIt)->width();
            int h = (*sizeIt)->height();
            QString page = namePage( w, h, current );
            QString text = (*sizeIt)->text(false);
            resolutions += QString::fromLatin1( " " );

            if ( width == w && height == h )
                resolutions += text;
            else
                resolutions += QString::fromLatin1( "<a href=\"%1\">%2</a>" ).arg( page ).arg( text );
        }
    }
    content.replace( QString::fromLatin1( "**RESOLUTIONS**" ), resolutions );

    // -------------------------------------------------- Description
    QString description;

    QValueList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    for( QValueList<DB::CategoryPtr>::Iterator it = categories.begin(); it != categories.end(); ++it ) {
        if ( (*it)->isSpecialCategory() )
            continue;

        QString name = (*it)->name();
        if ( info->itemsOfCategory( name ).count() != 0 && _setup.includeCategory(name) ) {
            QString val = QStringList(info->itemsOfCategory( name ).toList()).join( QString::fromLatin1(", ") );
            description += QString::fromLatin1("  <li> <b>%1:</b> %2\n").arg( name ).arg( val );
        }
    }

    if ( !info->description().isEmpty() && _setup.includeCategory( QString::fromLatin1( "**DESCRIPTION**" )) ) {
        description += QString::fromLatin1( "  <li> <b>Description:</b> %1\n" ).arg( info->description() );
    }

    if ( !description.isEmpty() )
        content.replace( QString::fromLatin1( "**DESCRIPTION**" ), QString::fromLatin1( "<ul>\n%1\n</ul>" ).arg( description ) );
    else
        content.replace( QString::fromLatin1( "**DESCRIPTION**" ), QString::fromLatin1( "" ) );

    // -------------------------------------------------- write to file
    QString fileName = _tempDir + namePage( width, height, current );
    bool ok = writeToFile( fileName, content );
    if ( !ok )
        return false;

    return true;
}

QString HTMLGenerator::Generator::namePage( int width, int height, const QString& fileName )
{
    QString name = _nameMap[fileName];
    QString base = QFileInfo( name ).baseName(true);
    return QString::fromLatin1( "%1-%2.html" ).arg( base ).arg( ImageSizeCheckBox::text(width,height,true) );
}

QString HTMLGenerator::Generator::nameImage( const QString& fileName, int size )
{
    QString name = _nameMap[fileName];
    QString base = QFileInfo( name ).baseName(true);
    if ( size == maxImageSize() && !Utilities::isVideo( fileName ) )
        return name;
    else
        return QString::fromLatin1( "%1-%2.jpg" ).arg( base ).arg( size );
}

QString HTMLGenerator::Generator::createImage( const QString& fileName, int size )
{
    if ( _generatedFiles.contains( qMakePair(fileName,size) ) ) {
        _waitCounter--;
    }
    else {
        ImageManager::ImageRequest* request =
            new ImageManager::ImageRequest( fileName, QSize( size, size ), DB::ImageDB::instance()->info(fileName)->angle(), this );
        request->setPriority();
        ImageManager::Manager::instance()->load( request );
        _generatedFiles.insert( qMakePair( fileName, size ) );
    }

    return nameImage( fileName, size );
}

QString HTMLGenerator::Generator::createVideo( const QString& fileName )
{
    setProgress( _total - _waitCounter );
    qApp->processEvents();

    QString baseName = QFileInfo(fileName).fileName();
    QString destName = _tempDir+QString::fromLatin1("/") + baseName;
    if ( !_copiedVideos.contains( fileName )) {
        Utilities::copy( fileName, destName );
        _copiedVideos.insert( fileName );
    }
    return baseName;
}

QString HTMLGenerator::Generator::kimFileName( bool relative )
{
    if ( relative )
        return QString::fromLatin1( "%2.kim" ).arg( _setup.outputDir() );
    else
        return QString::fromLatin1( "%1/%2.kim" ).arg( _tempDir ).arg( _setup.outputDir() );
}

bool HTMLGenerator::Generator::writeToFile( const QString& fileName, const QString& str )
{
    QFile file(fileName);
    if ( !file.open(IO_WriteOnly) ) {
        KMessageBox::error( this, i18n("Could not create file '%1'.").arg(fileName),
                            i18n("Could Not Create File") );
        return false;
    }

    QCString cstr = translateToHTML(str).utf8();
    file.writeBlock( cstr.data(), cstr.size() - 1);
    file.close();
    return true;
}


QString HTMLGenerator::Generator::translateToHTML( const QString& str )
{
    QString res;
    for ( uint i = 0 ; i < str.length() ; ++i ) {
        if ( str[i].unicode() < 128 )
            res.append( str[i] );
        else {
            res.append( QString().sprintf("&#%u;", (unsigned int)str[i].unicode() ) );
        }
    }
    return res;
}

bool HTMLGenerator::Generator::linkIndexFile()
{
    ImageSizeCheckBox* resolution = _setup.activeResolutions()[0];
    QString fromFile = QString::fromLatin1("index-%1.html" )
                       .arg(resolution->text(true));
    QString destFile = _tempDir + QString::fromLatin1("/index.html");
    bool ok = Utilities::copy( QFileInfo(destFile).dirPath() + fromFile, destFile );
    if ( !ok ) {
        KMessageBox::error( this, i18n("<p>Unable to copy %1 to %2</p>")
                            .arg( fromFile ).arg( destFile ) );

        return false;
    }
    return ok;
}

void HTMLGenerator::Generator::slotCancelGenerate()
{
    ImageManager::Manager::instance()->stop( this );
    _waitCounter = 0;
    if ( _hasEnteredLoop )
        qApp->eventLoop()->exitLoop();
}

void HTMLGenerator::Generator::pixmapLoaded( const QString& fileName, const QSize& imgSize,
                                     const QSize& /*fullSize*/, int /*angle*/, const QImage& image, bool loadedOK )
{
    setProgress( _total - _waitCounter );

    _waitCounter--;

    int size = imgSize.width();
    QString file = _tempDir + QString::fromLatin1( "/" ) + nameImage( fileName, size );

    bool success = loadedOK && image.save( file, "JPEG" );
    if ( !success ) {
        // We better stop the imageloading. In case this is a full disk, we will just get all images loaded, while this
        // error box is showing, resulting in a bunch of error messages, and memory running out due to all the hanging
        // pixmapLoaded methods.
        slotCancelGenerate();
        KMessageBox::error( this, i18n("Unable to write image '%1'.").arg(file) );
    }

#ifdef HASEXIV2
    if ( !Utilities::isVideo( fileName ) ) {
        try {
            Exif::Info::instance()->writeInfoToFile( fileName, file );
        }
        catch (...)
        {
        }
    }
#endif

    if ( _waitCounter == 0 && _hasEnteredLoop) {
        qApp->eventLoop()->exitLoop();
    }
}

int HTMLGenerator::Generator::calculateSteps()
{
    int count = _setup.activeResolutions().count();
    return _setup.imageList().count() * ( 1 + count ); // 1 thumbnail + 1 real image
}

void HTMLGenerator::Generator::getThemeInfo( QString* baseDir, QString* name, QString* author )
{
    *baseDir = _setup.themePath();
    KSimpleConfig themeConfig( QString::fromLatin1( "%1kphotoalbum.theme" ).arg( *baseDir ), true );
    themeConfig.setGroup( QString::fromLatin1( "theme" ) );
    *name = themeConfig.readEntry( "Name" );
    *author = themeConfig.readEntry( "Author" );
}

int HTMLGenerator::Generator::maxImageSize()
{
    int res = 0;
    for( QValueList<ImageSizeCheckBox*>::ConstIterator sizeIt = _setup.activeResolutions().begin();
         sizeIt != _setup.activeResolutions().end(); ++sizeIt ) {
        res = QMAX( res, (*sizeIt)->width() );
    }
    return res;
}

void HTMLGenerator::Generator::showBrowser()
{
    if ( _setup.generateKimFile() )
        ImportExport::Export::showUsageDialog();

    if ( ! _setup.baseURL().isEmpty() )
        new KRun( KURL(QString::fromLatin1( "%1/%2/index.html" ).arg( _setup.baseURL() ).arg( _setup.outputDir()) ) );

    qApp->eventLoop()->exitLoop();
}


#include "Generator.moc"
