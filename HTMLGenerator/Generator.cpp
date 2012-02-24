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
#include "Generator.h"
#include <klocale.h>

#include <QFile>
#include <QApplication>
#include <QList>
#include "ImageManager/Manager.h"
#include <KFileDialog>
#include <KRun>
#include <KMessageBox>
#include "Utilities/Util.h"
#include <KDebug>
#include <QDir>
#include "ImportExport/Export.h"
#include "DB/CategoryCollection.h"
#include "DB/ImageInfo.h"
#include "DB/ImageDB.h"
#include <config-kpa-exiv2.h>
#ifdef HAVE_EXIV2
#  include "Exif/Info.h"
#endif
#include "ImageSizeCheckBox.h"
#include "Setup.h"
#include "MainWindow/Window.h"
#include <KIO/CopyJob>

HTMLGenerator::Generator::Generator( const Setup& setup, QWidget* parent )
    : QProgressDialog( parent ), _hasEnteredLoop( false )
{
    setLabelText( i18n("Generating images for HTML page ") );
    _setup = setup;
    _eventLoop = new QEventLoop;
}

HTMLGenerator::Generator::~Generator()
{
    delete _eventLoop;
}
void HTMLGenerator::Generator::generate()
{
    // Generate .kim file
    if ( _setup.generateKimFile() ) {
        bool ok;
        QString destURL = _setup.destURL();
        if ( destURL.isEmpty() )
            destURL = _setup.baseURL();

        ImportExport::Export exp( _setup.imageList(), kimFileName( false ),
                                  false, -1, ImportExport::ManualCopy,
                                  destURL + QString::fromLatin1("/") + _setup.outputDir(), true, &ok);
        if ( !ok )
            return;
    }

    // prepare the progress dialog
    _total = _waitCounter = calculateSteps();
    setMaximum( _total );
    setValue( 0 );
    connect( this, SIGNAL( canceled() ), this, SLOT( slotCancelGenerate() ) );

    _filenameMapper.reset();

    // Itertate over each of the image sizes needed.
     for( QList<ImageSizeCheckBox*>::ConstIterator sizeIt = _setup.activeResolutions().begin();
         sizeIt != _setup.activeResolutions().end(); ++sizeIt ) {

        bool ok = generateIndexPage( (*sizeIt)->width(), (*sizeIt)->height() );
        if ( !ok )
            return;
        const DB::IdList& imageList = _setup.imageList();
        for (int index = 0; index < imageList.size(); ++index) {
            DB::Id current = imageList.at(index);
            DB::Id prev;
            DB::Id next;
            if ( index != 0 )
                prev = imageList.at(index - 1);
            if (index != imageList.size() - 1)
                next = imageList.at(index + 1);
            ok = generateContentPage( (*sizeIt)->width(), (*sizeIt)->height(),
                                      prev, current, next );
            if (!ok)
                return;
        }
    }

    // Now generate the thumbnail images
    Q_FOREACH(DB::Id id, _setup.imageList()) {
        if ( wasCanceled() )
            return;

        createImage(id, _setup.thumbSize());
    }

    if ( wasCanceled() )
        return;

    if ( _waitCounter > 0 ) {
        _hasEnteredLoop = true;
        _eventLoop->exec();
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
        kDebug() << QString::fromLatin1("theme '%1' doesn't have enough files to be a theme").arg( themeDir );

    for( QStringList::Iterator it = files.begin(); it != files.end(); ++it ) {
        if( *it == QString::fromLatin1("kphotoalbum.theme") ||
            *it == QString::fromLatin1("mainpage.html") ||
            *it == QString::fromLatin1("imagepage.html")) continue;
        QString from = QString::fromLatin1("%1%2").arg( themeDir ).arg(*it);
        QString to = _tempDir.name() + QString::fromLatin1("/") + *it;
        ok = Utilities::copy( from, to );
        if ( !ok ) {
            KMessageBox::error( this, i18n("Error copying %1 to %2", from , to ) );
            return;
        }
    }


    // Copy files over to destination.
    QString outputDir = _setup.baseDir() + QString::fromLatin1( "/" ) + _setup.outputDir();
    KIO::CopyJob* job = KIO::move( KUrl( _tempDir.name() ), KUrl(outputDir) );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( showBrowser() ) );

    _eventLoop->exec();
    return;
}

bool HTMLGenerator::Generator::generateIndexPage( int width, int height )
{
    QString themeDir, themeAuthor, themeName;
    getThemeInfo( &themeDir, &themeName, &themeAuthor );
    QString content = Utilities::readFile( QString::fromLatin1( "%1mainpage.html" ).arg( themeDir ) );
    if ( content.isEmpty() )
        return false;

    // Adding the copyright comment after DOCTYPE not before (HTML standard requires the DOCTYPE to be first within the document)
    QRegExp rx( QString::fromLatin1( "^(<!DOCTYPE[^>]*>)" ) );
    int position;
 
    rx.setCaseSensitivity( Qt::CaseInsensitive );
    position = rx.indexIn( content );
    if ( ( position += rx.matchedLength () ) < 0 )
	content = QString::fromLatin1("<!--\nMade with KPhotoAlbum. (http://www.kphotoalbum.org/)\nCopyright &copy; Jesper K. Pedersen\nTheme %1 by %2\n-->\n").arg( themeName ).arg( themeAuthor ) + content;
    else
	content.insert( position, QString::fromLatin1("\n<!--\nMade with KPhotoAlbum. (http://www.kphotoalbum.org/)\nCopyright &copy; Jesper K. Pedersen\nTheme %1 by %2\n-->\n").arg( themeName ).arg( themeAuthor ) );

    content.replace( QString::fromLatin1( "**DESCRIPTION**" ), _setup.description() );
    content.replace( QString::fromLatin1( "**TITLE**" ), _setup.title() );

    QString copyright;
    if (!_setup.copyright().isEmpty())
        copyright = QString::fromLatin1( "&#169; %1" ).arg( _setup.copyright() );
    else
        copyright = QString::fromLatin1( "&nbsp;" );
    content.replace( QString::fromLatin1( "**COPYRIGHT**" ), copyright );

    QString kimLink = QString::fromLatin1( "Share and Enjoy <a href=\"%1\">KPhotoAlbum export file</a>" ).arg( kimFileName( true ) );
    if ( _setup.generateKimFile() )
        content.replace( QString::fromLatin1( "**KIMFILE**" ), kimLink );
    else
        content.remove( QString::fromLatin1( "**KIMFILE**" ) );
    QDomDocument doc;

    QDomElement elm;
    QDomElement col;

    // -------------------------------------------------- Thumbnails
    // Initially all of the HTML generation was done using QDom, but it turned out in the end
    // to be much less code simply concatenating strings. This part, however, is easier using QDom
    // so we keep it using QDom.
    int count = 0;
    int cols = _setup.numOfCols();
    int minWidth = 0;
    int minHeight = 0;
    QString first, last, images;

    images += QString::fromLatin1( "var gallery=new Array()\nvar width=%1\nvar height=%2\nvar tsize=%3\n" ).arg( width ).arg( height ).arg( _setup.thumbSize() );

    minImageSize(minWidth, minHeight);
    images += QString::fromLatin1( "var minPage=\"index-%1x%2.html\"\n" ).arg( minWidth ).arg( minHeight );

    QDomElement row;
    Q_FOREACH(const DB::ImageInfoPtr info, _setup.imageList().fetchInfos()) {
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

        const QString fileName = info->fileName(DB::AbsolutePath);

        if (first.isEmpty())
            first = namePage( width, height, fileName);
        else
            last = namePage( width, height, fileName);

	if (!Utilities::isVideo(fileName))
            images += QString::fromLatin1( "gallery.push([\"%1\", \"%2\", \"%3\", \"" )
                  .arg( nameImage( fileName, width ) ).arg( nameImage( fileName, _setup.thumbSize() ) ).arg( nameImage( fileName, maxImageSize() ) );
	else
            images += QString::fromLatin1( "gallery.push([\"%1\", \"%2\", \"%3\", \"" )
                  .arg( nameImage( fileName, _setup.thumbSize() ) ).arg( nameImage( fileName, _setup.thumbSize() ) ).arg( QFileInfo(fileName).fileName() );

        // -------------------------------------------------- Description
        QString description = populateDescription(DB::ImageDB::instance()->categoryCollection()->categories(), info);

        if ( !description.isEmpty() )
            description = QString::fromLatin1 ( "<ul>%1</ul>" ).arg ( description );
        else
            description = QString::fromLatin1 ( "" );

        description.replace( QString::fromLatin1( "\n$" ), QString::fromLatin1 ( "" ) );
        description.replace( QString::fromLatin1( "\n" ), QString::fromLatin1 ( " " ) );
        description.replace( QString::fromLatin1( "\"" ), QString::fromLatin1 ( "\\\"" ) );

        images += description;
        images += QString::fromLatin1( "\"]);\n" );

        QDomElement href = doc.createElement( QString::fromLatin1( "a" ) );
        href.setAttribute( QString::fromLatin1( "href" ),
                           namePage( width, height, fileName));
        col.appendChild( href );

        QDomElement img = doc.createElement( QString::fromLatin1( "img" ) );
        img.setAttribute( QString::fromLatin1( "src" ),
                          nameImage( fileName, _setup.thumbSize() ) );
        img.setAttribute( QString::fromLatin1( "alt" ),
                          nameImage( fileName, _setup.thumbSize() ) );
        href.appendChild( img );
        ++count;
    }
    
    // Adding TD elements to match the selected column amount for valid HTML
    if ( count % cols != 0 ) {
	for ( int i = count; i % cols != 0; ++i ) {
	    col = doc.createElement( QString::fromLatin1( "td" ) );
	    col.setAttribute( QString::fromLatin1( "class" ), QString::fromLatin1( "thumbnail-col" ) );
	    QDomText sp =  doc.createTextNode( QString::fromLatin1( " " ) );
	    col.appendChild( sp );
	    row.appendChild( col );
	}
    }

    content.replace( QString::fromLatin1( "**THUMBNAIL-TABLE**" ), doc.toString() );

    content.replace( QString::fromLatin1( "**JSIMAGES**" ), images );
    if (!first.isEmpty())
	content.replace( QString::fromLatin1( "**FIRST**" ), first );
    if (!last.isEmpty())
	content.replace( QString::fromLatin1( "**LAST**" ), last );

    // -------------------------------------------------- Resolutions
    QString resolutions;
    QList<ImageSizeCheckBox*> actRes = _setup.activeResolutions();
    qSort(actRes);

    if ( actRes.count() > 1 ) {
        resolutions += QString::fromLatin1( "Resolutions: " );
        for( QList<ImageSizeCheckBox*>::ConstIterator sizeIt = actRes.constBegin();
             sizeIt != actRes.constEnd(); ++sizeIt ) {

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
    QString fileName = _tempDir.name() + QString::fromLatin1("/index-%1.html" )
                       .arg(ImageSizeCheckBox::text(width,height,true));
    bool ok = writeToFile( fileName, content );

    if ( !ok )
        return false;

    return true;
}

bool HTMLGenerator::Generator::generateContentPage( int width, int height,
                                                    const DB::Id& prev, const DB::Id& current, const DB::Id& next )
{
    QString themeDir, themeAuthor, themeName;
    getThemeInfo( &themeDir, &themeName, &themeAuthor );
    QString content = Utilities::readFile( QString::fromLatin1( "%1imagepage.html" ).arg( themeDir ));
    if ( content.isEmpty() )
        return false;

    DB::ImageInfoPtr info = current.fetchInfo();
    QString currentFile = info->fileName(DB::AbsolutePath);

    // Adding the copyright comment after DOCTYPE not before (HTML standard requires the DOCTYPE to be first within the document)
    QRegExp rx( QString::fromLatin1( "^(<!DOCTYPE[^>]*>)" ) );
    int position;
 
    rx.setCaseSensitivity( Qt::CaseInsensitive );
    position = rx.indexIn( content );
    if ( ( position += rx.matchedLength () ) < 0 )
	content = QString::fromLatin1("<!--\nMade with KPhotoAlbum. (http://www.kphotoalbum.org/)\nCopyright &copy; Jesper K. Pedersen\nTheme %1 by %2\n-->\n").arg( themeName ).arg( themeAuthor ) + content;
    else
	content.insert( position, QString::fromLatin1("\n<!--\nMade with KPhotoAlbum. (http://www.kphotoalbum.org/)\nCopyright &copy; Jesper K. Pedersen\nTheme %1 by %2\n-->\n").arg( themeName ).arg( themeAuthor ) );

    content.replace( QString::fromLatin1( "**TITLE**" ), info->label() );


    // Image or video content
    if (Utilities::isVideo(currentFile)) {
        QString videoFile = createVideo( currentFile );
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
        link = i18n( "<a href=\"%1\">prev</a>", namePage( width, height, prev.fetchInfo()->fileName(DB::AbsolutePath)));
    else
        link = i18n( "prev" );
    content.replace( QString::fromLatin1( "**PREV**" ), link );

    // PENDING(blackie) These next 5 line also exists exactly like that in HTMLGenerator::Generator::generateIndexPage. Please refactor.
    // prevfile
    if ( !prev.isNull() )
        link = namePage( width, height, prev.fetchInfo()->fileName(DB::AbsolutePath));
    else
        link = i18n( "prev" );
    content.replace( QString::fromLatin1( "**PREVFILE**" ), link );

    // index link
    link = i18n( "<a href=\"index-%1.html\">index</a>", ImageSizeCheckBox::text(width,height,true));
    content.replace( QString::fromLatin1( "**INDEX**" ), link );

    // indexfile
    link = QString::fromLatin1( "index-%1.html").arg(ImageSizeCheckBox::text(width,height,true));
    content.replace( QString::fromLatin1( "**INDEXFILE**" ), link );

    // Next Link
    if ( !next.isNull() )
        link = i18n( "<a href=\"%1\">next</a>", namePage( width, height, next.fetchInfo()->fileName(DB::AbsolutePath)));
    else
        link = i18n( "next" );
    content.replace( QString::fromLatin1( "**NEXT**" ), link );

    // Nextfile
    if ( !next.isNull() )
        link = namePage( width, height, next.fetchInfo()->fileName(DB::AbsolutePath));
    else
        link = i18n( "next" );
    content.replace( QString::fromLatin1( "**NEXTFILE**" ), link );

    if ( !next.isNull() )
        link = namePage( width, height, next.fetchInfo()->fileName(DB::AbsolutePath) );
    else
        link = QString::fromLatin1( "index-%1.html" ).arg(ImageSizeCheckBox::text(width,height,true));

    content.replace( QString::fromLatin1( "**NEXTPAGE**" ), link );


    // -------------------------------------------------- Resolutions
    QString resolutions;
     const QList<ImageSizeCheckBox*>& actRes = _setup.activeResolutions();
    if ( actRes.count() > 1 ) {
         for( QList<ImageSizeCheckBox*>::ConstIterator sizeIt = actRes.begin();
             sizeIt != actRes.end(); ++sizeIt ) {
            int w = (*sizeIt)->width();
            int h = (*sizeIt)->height();
            QString page = namePage( w, h, currentFile );
            QString text = (*sizeIt)->text(false);
            resolutions += QString::fromLatin1( " " );

            if ( width == w && height == h )
                resolutions += text;
            else
                resolutions += QString::fromLatin1( "<a href=\"%1\">%2</a>" ).arg( page ).arg( text );
        }
    }
    content.replace( QString::fromLatin1( "**RESOLUTIONS**" ), resolutions );

    // -------------------------------------------------- Copyright
    QString copyright;

    if ( !_setup.copyright().isEmpty() )
        copyright = QString::fromLatin1( "&#169; %1" ).arg( _setup.copyright() );
    else
        copyright = QString::fromLatin1( "&nbsp;" );
    content.replace( QString::fromLatin1( "**COPYRIGHT**" ), QString::fromLatin1( "%1" ).arg( copyright ) );


    // -------------------------------------------------- Description
    QString description = populateDescription(DB::ImageDB::instance()->categoryCollection()->categories(), info);

    if ( !description.isEmpty() )
        content.replace( QString::fromLatin1( "**DESCRIPTION**" ), QString::fromLatin1( "<ul>\n%1\n</ul>" ).arg( description ) );
    else
        content.replace( QString::fromLatin1( "**DESCRIPTION**" ), QString::fromLatin1( "" ) );

    // -------------------------------------------------- write to file
    QString fileName = _tempDir.name() + namePage( width, height, currentFile );
    bool ok = writeToFile( fileName, content );
    if ( !ok )
        return false;

    return true;
}

QString HTMLGenerator::Generator::namePage( int width, int height, const QString& fileName )
{
    QString name = _filenameMapper.uniqNameFor(fileName);
    QString base = QFileInfo( name ).completeBaseName();
    return QString::fromLatin1( "%1-%2.html" ).arg( base ).arg( ImageSizeCheckBox::text(width,height,true) );
}

QString HTMLGenerator::Generator::nameImage( const QString& fileName, int size )
{
    QString name = _filenameMapper.uniqNameFor(fileName);
    QString base = QFileInfo( name ).completeBaseName();
    if ( size == maxImageSize() && !Utilities::isVideo( fileName ) )
        if ( name.endsWith( QString::fromAscii(".jpg"), Qt::CaseSensitive ) ||
                name.endsWith( QString::fromAscii(".jpeg"), Qt::CaseSensitive ) )
            return name;
        else
            return base + QString::fromAscii(".jpg");
    else
        return QString::fromLatin1( "%1-%2.jpg" ).arg( base ).arg( size );
}

QString HTMLGenerator::Generator::createImage( const DB::Id& id, int size )
{
    DB::ImageInfoPtr info = id.fetchInfo();
    const QString fileName = info->fileName(DB::AbsolutePath);
    if ( _generatedFiles.contains( qMakePair(fileName,size) ) ) {
        _waitCounter--;
    }
    else {
        ImageManager::ImageRequest* request =
            new ImageManager::ImageRequest( fileName, QSize( size, size ),
                                            info->angle(), this );
        request->setPriority( ImageManager::BatchTask );
        ImageManager::Manager::instance()->load( request );
        _generatedFiles.insert( qMakePair( fileName, size ) );
    }

    return nameImage( fileName, size );
}

QString HTMLGenerator::Generator::createVideo( const QString& fileName )
{
    setValue( _total - _waitCounter );
    qApp->processEvents();

    QString baseName = QFileInfo(fileName).fileName();
    QString destName = _tempDir.name() + QString::fromLatin1("/") + baseName;
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
        return QString::fromLatin1( "%1/%2.kim" ).arg( _tempDir.name() ).arg( _setup.outputDir() );
}

bool HTMLGenerator::Generator::writeToFile( const QString& fileName, const QString& str )
{
    QFile file(fileName);
    if ( !file.open(QIODevice::WriteOnly) ) {
        KMessageBox::error( this, i18n("Could not create file '%1'.",fileName),
                            i18n("Could Not Create File") );
        return false;
    }

    QByteArray data = translateToHTML(str).toUtf8();
    file.write( data );
    file.close();
    return true;
}


QString HTMLGenerator::Generator::translateToHTML( const QString& str )
{
    QString res;
    for ( int i = 0 ; i < str.length() ; ++i ) {
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
    QString destFile = _tempDir.name() + QString::fromLatin1("/index.html");
    bool ok = Utilities::copy( QFileInfo(destFile).path() + fromFile, destFile );
    if ( !ok ) {
        KMessageBox::error( this, i18n("<p>Unable to copy %1 to %2</p>"
                            , fromFile , destFile ) );

        return false;
    }
    return ok;
}

void HTMLGenerator::Generator::slotCancelGenerate()
{
    ImageManager::Manager::instance()->stop( this );
    _waitCounter = 0;
    if ( _hasEnteredLoop )
        _eventLoop->exit();
}

void HTMLGenerator::Generator::pixmapLoaded( const QString& fileName, const QSize& imgSize,
                                             const QSize& /*fullSize*/, int /*angle*/, const QImage& image, const bool loadedOK)
{
    setValue( _total - _waitCounter );

    _waitCounter--;

    int size = imgSize.width();
    QString file = _tempDir.name() + QString::fromLatin1( "/" ) + nameImage( fileName, size );

    bool success = loadedOK && image.save( file, "JPEG" );
    if ( !success ) {
        // We better stop the imageloading. In case this is a full disk, we will just get all images loaded, while this
        // error box is showing, resulting in a bunch of error messages, and memory running out due to all the hanging
        // pixmapLoaded methods.
        slotCancelGenerate();
        KMessageBox::error( this, i18n("Unable to write image '%1'.",file) );
    }

#ifdef HAVE_EXIV2
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
        _eventLoop->exit();
    }
}

int HTMLGenerator::Generator::calculateSteps()
{
    int count = _setup.activeResolutions().count();
    return _setup.imageList().size() * (1 + count); // 1 thumbnail + 1 real image
}

void HTMLGenerator::Generator::getThemeInfo( QString* baseDir, QString* name, QString* author )
{
    *baseDir = _setup.themePath();
    KConfig themeconfig( QString::fromLatin1( "%1/kphotoalbum.theme").arg( *baseDir ), KConfig::SimpleConfig );
    KConfigGroup config = themeconfig.group("theme");

    *name = config.readEntry( "Name" );
    *author = config.readEntry( "Author" );
}

int HTMLGenerator::Generator::maxImageSize()
{
    int res = 0;
     for( QList<ImageSizeCheckBox*>::ConstIterator sizeIt = _setup.activeResolutions().begin();
         sizeIt != _setup.activeResolutions().end(); ++sizeIt ) {
        res = qMax( res, (*sizeIt)->width() );
    }
    return res;
}

void HTMLGenerator::Generator::minImageSize(int& width, int& height)
{
    width = height = 0;
    for( QList<ImageSizeCheckBox*>::ConstIterator sizeIt = _setup.activeResolutions().begin();
         sizeIt != _setup.activeResolutions().end(); ++sizeIt ) {
	if ((width == 0) && ((*sizeIt)->width() > 0)) {
	    width = (*sizeIt)->width();
	    height = (*sizeIt)->height();
	} else if ((*sizeIt)->width() > 0) {
            width = qMin( width, (*sizeIt)->width() );
	    height = qMin( height, (*sizeIt)->height());
	}
    }
}

void HTMLGenerator::Generator::showBrowser()
{
    if ( _setup.generateKimFile() )
        ImportExport::Export::showUsageDialog();

    if ( ! _setup.baseURL().isEmpty() )
        new KRun( KUrl(QString::fromLatin1( "%1/%2/index.html" ).arg( _setup.baseURL() ).arg( _setup.outputDir()) ),
                       MainWindow::Window::theMainWindow());

    _eventLoop->exit();
}

QString HTMLGenerator::Generator::populateDescription( QList<DB::CategoryPtr> categories, const DB::ImageInfoPtr info )
{
     QString description;

    if (_setup.includeCategory(QString::fromLatin1("**DATE**")))
	description += QString::fromLatin1 ( "<li> <b>%1</b> %2</li>" ).arg ( i18n("Date") ).arg ( info->date().toString() );

     for( QList<DB::CategoryPtr>::Iterator it = categories.begin(); it != categories.end(); ++it ) {
        if ( (*it)->isSpecialCategory() )
            continue;

        QString name = (*it)->name();
        if ( !info->itemsOfCategory( name ).empty() && _setup.includeCategory(name) ) {
            QString val = QStringList(info->itemsOfCategory( name ).toList()).join( QString::fromLatin1(", ") );
            description += QString::fromLatin1("  <li> <b>%1:</b> %2</li>").arg( name ).arg( val );
        }
    }

    if ( !info->description().isEmpty() && _setup.includeCategory( QString::fromLatin1( "**DESCRIPTION**" )) ) {
        description += QString::fromLatin1( "  <li> <b>Description:</b> %1</li>" ).arg( info->description() );
    }

    return description;
}
#include "Generator.moc"
