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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kdeversion.h"
#include "HtmlExportDialog.h"
#include <klocale.h>
#include <qlayout.h>
#include <klineedit.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qdir.h>
#include <qfile.h>
#include <qapplication.h>
#include <qeventloop.h>
#include "ImageManager/Manager.h"
#include <qcheckbox.h>
#include <kfiledialog.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include "Settings/SettingsData.h"
#include <qprogressdialog.h>
#include <qslider.h>
#include <qlcdnumber.h>
#include <qhgroupbox.h>
#include <kstandarddirs.h>
#include <krun.h>
#include <kio/job.h>
#include <ktempdir.h>
#include <kmessagebox.h>
#include <kfileitem.h>
#include <kio/netaccess.h>
#include <kio/jobclasses.h>
#include <qtextedit.h>
#include <qregexp.h>
#include <unistd.h>
#include "Utilities/Util.h"
#include <kdebug.h>
#include <qdir.h>
#include <ksimpleconfig.h>
#include <qvgroupbox.h>
#include <kglobal.h>
#include <kiconloader.h>
#include "ImportExport/Export.h"
#include "MainWindow/Window.h"
#include "DB/CategoryCollection.h"
#include "DB/ImageInfo.h"
#include "DB/ImageDB.h"
#include <config.h>
#ifdef HASEXIV2
#  include "Exif/Info.h"
#endif

using namespace MainWindow;

class ImageSizeCheckBox :public QCheckBox {

public:
    ImageSizeCheckBox( int width, int height, QWidget* parent )
        :QCheckBox( QString::fromLatin1("%1x%2").arg(width).arg(height), parent ),
         _width( width ), _height( height )
        {
        }

    ImageSizeCheckBox( const QString& text, QWidget* parent )
        :QCheckBox( text, parent ), _width( -1 ), _height( -1 )
        {
        }

    int width() const {
        return _width;
    }
    int height() const {
        return _height;
    }
    QString text( bool withOutSpaces ) const {
        return text( _width, _height, withOutSpaces );
    }
    static QString text( int width, int height, bool withOutSpaces ) {
        if ( width == -1 )
            if ( withOutSpaces )
                return QString::fromLatin1("fullsize");
            else
                return QString::fromLatin1("full size");

        else
            return QString::fromLatin1("%1x%2").arg(width).arg(height);
    }


private:
    int _width;
    int _height;
};

HTMLExportDialog::HTMLExportDialog( QWidget* parent, const char* name )
    :KDialogBase( IconList, i18n("HTML Export"), Ok|Cancel|Help, Ok, parent, name )
{
    enableButtonOK( false );
    createContentPage();
    createLayoutPage();
    createDestinationPage();
    setHelp( QString::fromLatin1( "chp-generating-html" ) );
}

void HTMLExportDialog::createContentPage()
{
    QWidget* contentPage = addPage( i18n("Content" ), i18n("Content" ),
                                    KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "edit" ),
                                                                     KIcon::Desktop, 32 ));
    QVBoxLayout* lay1 = new QVBoxLayout( contentPage, 6 );
    QGridLayout* lay2 = new QGridLayout( lay1, 2 );

    QLabel* label = new QLabel( i18n("Page title:"), contentPage );
    lay2->addWidget( label, 0, 0 );
    _title = new KLineEdit( contentPage );
    lay2->addWidget( _title, 0, 1 );

    // Description
    label = new QLabel( i18n("Description:"), contentPage );
    label->setAlignment( Qt::AlignTop );
    lay2->addWidget( label, 1, 0 );
    _description = new QTextEdit( contentPage );
    lay2->addWidget( _description, 1, 1 );

    _generateKimFile = new QCheckBox( i18n("Create .kim export file"), contentPage );
    _generateKimFile->setChecked( true );
    lay1->addWidget( _generateKimFile );

    // What to include
    QVGroupBox* whatToInclude = new QVGroupBox( i18n( "What to Include" ), contentPage );
    lay1->addWidget( whatToInclude );
    QWidget* w = new QWidget( whatToInclude );
    QGridLayout* lay3 = new QGridLayout( w, 1, 2, 6 );
    lay3->setAutoAdd( true );

    QValueList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    for( QValueList<DB::CategoryPtr>::Iterator it = categories.begin(); it != categories.end(); ++it ) {
        if ( ! (*it)->isSpecialCategory() ) {
            QCheckBox* cb = new QCheckBox( (*it)->text(), w );
            _whatToIncludeMap.insert( (*it)->name(), cb );
        }
    }
    QCheckBox* cb = new QCheckBox( i18n("Description"), w );
    _whatToIncludeMap.insert( QString::fromLatin1("**DESCRIPTION**"), cb );
}

void HTMLExportDialog::createLayoutPage()
{
    QWidget* layoutPage = addPage( i18n("Layout" ), i18n("Layout" ),
                                   KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "matrix" ),
                                                                    KIcon::Desktop, 32 ));
    QVBoxLayout* lay1 = new QVBoxLayout( layoutPage, 6 );
    QGridLayout* lay2 = new QGridLayout( lay1, 2, 2, 6 );

    // Thumbnail size
    QLabel* label = new QLabel( i18n("Thumbnail size:"), layoutPage );
    lay2->addWidget( label, 0, 0 );

    QHBoxLayout* lay3 = new QHBoxLayout( 0 );
    lay2->addLayout( lay3, 0, 1 );

    _thumbSize = new QSpinBox( 16, 256, 1, layoutPage );
    _thumbSize->setValue( 128 );
    lay3->addWidget( _thumbSize );
    lay3->addStretch(1);

    // Number of columns
    label = new QLabel( i18n("Number of columns:"), layoutPage );
    lay2->addWidget( label, 1, 0 );

    QHBoxLayout* lay4 = new QHBoxLayout( 0 );
    lay2->addLayout( lay4, 1, 1 );
    _numOfCols = new QSpinBox( 1, 10, 1, layoutPage );
    _numOfCols->setValue( 5 );
    lay4->addWidget( _numOfCols );
    lay4->addStretch( 1 );

    // Theme box
    label = new QLabel( i18n("Theme:"), layoutPage );
    lay2->addWidget( label, 2, 0 );
    lay4 = new QHBoxLayout( 0 );
    lay2->addLayout( lay4, 2, 1 );
    _themeBox = new QComboBox( layoutPage, "theme_combobox" );
    lay4->addWidget( _themeBox );
    lay4->addStretch( 1 );
    populateThemesCombo();

    // Image sizes
    QHGroupBox* sizes = new QHGroupBox( i18n("Image Sizes"), layoutPage );
    lay1->addWidget( sizes );
    QWidget* content = new QWidget( sizes );
    QGridLayout* lay5 = new QGridLayout( content, 2, 4 );
    lay5->setAutoAdd( true );
    ImageSizeCheckBox* size320  = new ImageSizeCheckBox( 320, 200, content );
    ImageSizeCheckBox* size640  = new ImageSizeCheckBox( 640, 480, content );
    ImageSizeCheckBox* size800  = new ImageSizeCheckBox( 800, 600, content );
    ImageSizeCheckBox* size1024 = new ImageSizeCheckBox( 1024, 768, content );
    ImageSizeCheckBox* size1280 = new ImageSizeCheckBox( 1280, 1024, content );
    ImageSizeCheckBox* size1600 = new ImageSizeCheckBox( 1600, 1200, content );
    ImageSizeCheckBox* sizeOrig = new ImageSizeCheckBox( i18n("Full size"), content );
    size800->setChecked( 1 );

    _cbs << size320 << size640 << size800 << size1024 << size1280 << size1600 << sizeOrig;
    _preferredSizes << size800 << size1024 << size1280 << size640 << size1600 << size320 << sizeOrig;

    lay1->addStretch(1);
}

void HTMLExportDialog::createDestinationPage()
{
    QWidget* destinationPage = addPage( i18n("Destination" ), i18n("Destination" ),
                                        KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "hdd_unmount" ),
                                                                         KIcon::Desktop, 32 ));
    QVBoxLayout* lay1 = new QVBoxLayout( destinationPage, 6 );
    QGridLayout* lay2 = new QGridLayout( lay1, 2 );

    // Base Directory
    QLabel* label = new QLabel( i18n("Base directory:"), destinationPage );
    lay2->addWidget( label, 0, 0 );

    QHBoxLayout* lay3 = new QHBoxLayout( (QWidget*)0, 0, 6 );
    lay2->addLayout( lay3, 0, 1 );

    _baseDir = new KLineEdit( destinationPage );
    lay3->addWidget( _baseDir );

    QPushButton* but = new QPushButton( QString::fromLatin1( ".." ), destinationPage );
    lay3->addWidget( but );
    but->setFixedWidth( 25 );

    connect( but, SIGNAL( clicked() ), this, SLOT( selectDir() ) );
    _baseDir->setText( Settings::SettingsData::instance()->HTMLBaseDir() );

    // Base URL
    label = new QLabel( i18n("Base URL:"), destinationPage );
    lay2->addWidget( label, 1, 0 );

    _baseURL = new KLineEdit( destinationPage );
    _baseURL->setText( Settings::SettingsData::instance()->HTMLBaseURL() );
    lay2->addWidget( _baseURL, 1, 1 );

    // Destination URL
    label = new QLabel( i18n("URL for final destination:" ), destinationPage );
    lay2->addWidget( label, 2, 0 );
    _destURL = new KLineEdit( destinationPage );
    _destURL->setText( Settings::SettingsData::instance()->HTMLDestURL() );
    lay2->addWidget( _destURL, 2, 1 );

    // Output Directory
    label = new QLabel( i18n("Output directory:"), destinationPage );
    lay2->addWidget( label, 3, 0 );
    _outputDir = new KLineEdit( destinationPage );
    lay2->addWidget( _outputDir, 3, 1 );

    label = new QLabel( i18n("<b>Hint: Press the help button for descriptions of the fields</b>"), destinationPage );
    lay1->addWidget( label );
    lay1->addStretch( 1 );
}

bool HTMLExportDialog::generate()
{
    if ( !checkVars() )
        return false;

    hide();

    _tempDir = KTempDir().name();


    // Generate .kim file
    if ( _generateKimFile->isChecked() ) {
        bool ok;
        QString destURL = _destURL->text();
        if ( destURL.isEmpty() )
            destURL = _baseURL->text();

        ImportExport::Export exp( _list, kimFileName( false ), false, -1, ImportExport::ManualCopy, destURL, ok, true );
        if ( !ok )
            return false;
    }

    // prepare the progress dialog
    _total = _waitCounter = calculateSteps();
    _progress->setTotalSteps( _total );
    _progress->setProgress( 0 );
    connect( _progress, SIGNAL( cancelled() ), this, SLOT( slotCancelGenerate() ) );

    _nameMap = Utilities::createUniqNameMap( _list, false, QString::null );

    // Itertate over each of the image sizes needed.
    for( QValueList<ImageSizeCheckBox*>::Iterator sizeIt = _cbs.begin(); sizeIt != _cbs.end(); ++sizeIt ) {
        if ( (*sizeIt)->isChecked() ) {
            bool ok = generateIndexPage( (*sizeIt)->width(), (*sizeIt)->height() );
            if ( !ok )
                return false;
            for ( uint index = 0; index < _list.count(); ++index ) {
                QString current = _list[index];
                QString prev;
                QString next;
                if ( index != 0 )
                    prev = _list[index-1];
                if ( index != _list.count() -1 )
                    next = _list[index+1];
                ok = generateContextPage( (*sizeIt)->width(), (*sizeIt)->height(), prev, current, next );
                if (!ok)
                    return false;
            }
        }
    }

    // Now generate the thumbnail images
    for( QStringList::Iterator it = _list.begin(); it != _list.end(); ++it ) {
        if ( _progress->wasCanceled() )
            return false;

        createImage( *it, _thumbSize->value() );
    }


    if ( _waitCounter > 0 )
        qApp->eventLoop()->enterLoop();

    bool ok = linkIndexFile();
    if ( !ok )
        return false;

    // Copy over the mainpage.css, indepage.css
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
            return false;
        }
    }


    // Copy files over to destination.
    QString outputDir = _baseDir->text() + QString::fromLatin1( "/" ) + _outputDir->text();
    KIO::CopyJob* job = KIO::move( KURL(_tempDir), KURL(outputDir) );
    connect( job, SIGNAL( result( KIO::Job* ) ), this, SLOT( showBrowser() ) );

    return true;
}

bool HTMLExportDialog::generateIndexPage( int width, int height )
{
    QString themeDir, themeAuthor, themeName;
    getThemeInfo( &themeDir, &themeName, &themeAuthor );
    QString content = Utilities::readFile( QString::fromLatin1( "%1mainpage.html" ).arg( themeDir ) );
    if ( content.isNull() )
        return false;

    content = QString::fromLatin1("<!--\nMade with KPhotoAlbum. (http://www.kphotoalbum.org/)\nCopyright &copy; Jesper K. Pedersen\nTheme %1 by %2\n-->\n").arg( themeName ).arg( themeAuthor ) + content;

    content.replace( QString::fromLatin1( "**DESCRIPTION**" ), _description->text() );
    content.replace( QString::fromLatin1( "**TITLE**" ), _title->text() );
    QString kimLink = QString::fromLatin1( "Share and Enjoy <a href=\"%1\">KPhotoAlbum export file</a>" ).arg( kimFileName( true ) );
    if ( _generateKimFile->isChecked() )
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
    int cols = _numOfCols->value();
    QDomElement row;
    for( QStringList::Iterator it = _list.begin(); it != _list.end(); ++it ) {
        if ( _progress->wasCanceled() )
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
                          nameImage( *it, _thumbSize->value() ) );
        img.setAttribute( QString::fromLatin1( "alt" ),
                          nameImage( *it, _thumbSize->value() ) );
        href.appendChild( img );
        ++count;
    }

    content.replace( QString::fromLatin1( "**THUMBNAIL-TABLE**" ), doc.toString() );

    // -------------------------------------------------- Resolutions
    QString resolutions;
    QValueList<ImageSizeCheckBox*> actRes = activeResolutions();
    if ( actRes.count() > 1 ) {
        resolutions += QString::fromLatin1( "Resolutions: " );
        for( QValueList<ImageSizeCheckBox*>::Iterator sizeIt = actRes.begin();
             sizeIt != actRes.end(); ++sizeIt ) {

            int w = (*sizeIt)->width();
            int h = (*sizeIt)->height();
            QString page = QString::fromLatin1( "index-%1.html" )
                           .arg( ImageSizeCheckBox::text( w, h, true ) );
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

    if ( _progress->wasCanceled() )
        return false;

    // -------------------------------------------------- write to file
    QString fileName = _tempDir + QString::fromLatin1("/index-%1.html" )
                       .arg(ImageSizeCheckBox::text(width,height,true));
    bool ok = writeToFile( fileName, content );

    if ( !ok )
        return false;

    return true;
}

bool HTMLExportDialog::generateContextPage( int width, int height, const QString& prev,
                                            const QString& current, const QString& next )
{
    QString themeDir, themeAuthor, themeName;
    getThemeInfo( &themeDir, &themeName, &themeAuthor );
    QString content = Utilities::readFile( QString::fromLatin1( "%1imagepage.html" ).arg( themeDir ));
    if ( content.isNull() )
        return false;

    DB::ImageInfoPtr info = DB::ImageDB::instance()->info( current );

    content = QString::fromLatin1("<!--\nMade with KPhotoAlbum. (http://www.kphotoalbum.org/)\nCopyright &copy; Jesper K. Pedersen\nTheme %1 by %2\n-->\n").arg( themeName ).arg( themeAuthor ) + content;

    content.replace( QString::fromLatin1( "**TITLE**" ), info->label() );
    content.replace( QString::fromLatin1( "**IMAGE**" ), createImage( current, width ) );


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
    QValueList<ImageSizeCheckBox*> actRes = activeResolutions();
    if ( actRes.count() > 1 ) {
        for( QValueList<ImageSizeCheckBox*>::Iterator sizeIt = actRes.begin();
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
        if ( info->itemsOfCategory( name ).count() != 0 && _whatToIncludeMap[name]->isChecked() ) {
            QString val = info->itemsOfCategory( name ).join( QString::fromLatin1(", ") );
            description += QString::fromLatin1("  <li> <b>%1:</b> %2\n").arg( name ).arg( val );
        }
    }

    if ( !info->description().isEmpty() && _whatToIncludeMap[QString::fromLatin1( "**DESCRIPTION**" )]->isChecked() ) {
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



bool HTMLExportDialog::writeToFile( const QString& fileName, const QString& str )
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


QString HTMLExportDialog::createImage( const QString& fileName, int size )
{
    ImageManager::ImageRequest* request =
        new ImageManager::ImageRequest( fileName, QSize( size, size ), DB::ImageDB::instance()->info(fileName)->angle(), this );
    request->setPriority();
    ImageManager::Manager::instance()->load( request );
    return nameImage( fileName, size );
}

void HTMLExportDialog::pixmapLoaded( const QString& fileName, const QSize& imgSize,
                                     const QSize& /*fullSize*/, int /*angle*/, const QImage& image, bool loadedOK )
{
    _progress->setProgress( _total - _waitCounter );

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
    Exif::Info::instance()->writeInfoToFile( fileName, file );
#endif

    if ( _waitCounter == 0 ) {
        qApp->eventLoop()->exitLoop();
    }
}

void HTMLExportDialog::slotOk()
{
    if( activeResolutions().count() < 1 ) {
        KMessageBox::error( 0, i18n( "You must select at least one resolution." ) );
        return;
    }
    // Progress dialog
    _progress = new QProgressDialog( i18n("Generating images for HTML page "), i18n("&Cancel"), 0, this );

    bool ok = generate();
    if ( ok ) {
        Settings::SettingsData::instance()->setHTMLBaseDir( _baseDir->text() );
        Settings::SettingsData::instance()->setHTMLBaseURL( _baseURL->text() );
        Settings::SettingsData::instance()->setHTMLDestURL( _destURL->text() );
        accept();
    }
    delete _progress;
    _progress = 0;
}

void HTMLExportDialog::selectDir()
{
    KURL dir = KFileDialog::getExistingURL( _baseDir->text(), this );
    if ( !dir.url().isNull() )
        _baseDir->setText( dir.url() );
}

void HTMLExportDialog::slotCancelGenerate()
{
    ImageManager::Manager::instance()->stop( this );
    _waitCounter = 0;
}

void HTMLExportDialog::showBrowser()
{
    if ( _generateKimFile->isChecked() )
        ImportExport::Export::showUsageDialog();

    if ( ! _baseURL->text().isEmpty() )
        new KRun( KURL(QString::fromLatin1( "%1/%2/index.html" ).arg( _baseURL->text() ).arg( _outputDir->text()) ) );
}

bool HTMLExportDialog::checkVars()
{
    QString outputDir = _baseDir->text() + QString::fromLatin1( "/" ) + _outputDir->text();


    // Ensure base dir is specified
    QString baseDir = _baseDir->text();
    if ( baseDir.isEmpty() ) {
        KMessageBox::error( this, i18n("<qt>You did not specify a base directory. "
                                       "This is the topmost directory for your images. "
                                       "Under this directory you will find each generated collection "
                                       "in separate directories.</qt>"),
                            i18n("No Base Directory Specified") );
        return false;
    }

    // ensure output directory is specified
    if ( _outputDir->text().isEmpty() ) {
        KMessageBox::error( this, i18n("<qt>You did not specify an output directory. "
                                       "This is a directory containing the actual images. "
                                       "The directory will be in the base directory specified above.</qt>"),
                            i18n("No Output Directory Specified") );
        return false;
    }

    // ensure base dir exists
    KIO::UDSEntry result;
#if KDE_IS_VERSION( 3,1,90 )
    bool ok = KIO::NetAccess::stat( KURL(baseDir), result, this );
#else
    bool ok = KIO::NetAccess::stat( KURL(baseDir), result );
#endif
    if ( !ok ) {
        KMessageBox::error( this, i18n("<qt>Error while reading information about %1. "
                                       "This is most likely because the directory does not exist.</qt>")
                            .arg( baseDir ) );
        return false;
    }

    KFileItem fileInfo( result, KURL(baseDir) );
    if ( !fileInfo.isDir() ) {
        KMessageBox::error( this, i18n("<qt>%1 does not exist, is not a directory or "
                                       "cannot be written to.</qt>").arg( baseDir ) );
        return false;
    }


    // test if destination directory exists.
#if KDE_IS_VERSION( 3, 1, 90 )
    bool exists = KIO::NetAccess::exists( KURL(outputDir), false, Window::theMainWindow() );
#else
    bool exists = KIO::NetAccess::exists( KURL(outputDir) );
#endif
    if ( exists ) {
        int answer = KMessageBox::warningYesNo( this,
                                                i18n("<qt><p>Output directory %1 already exists. "
                                                     "Usually you should specify a new directory.</p>"
                                                     "Should I delete %2 first?</qt>").arg( outputDir ).arg( outputDir ),
                                                i18n("Directory Exists"), KStdGuiItem::yes(), KStdGuiItem::no(),
                                                QString::fromLatin1("html_export_delete_original_directory") );
        if ( answer == KMessageBox::Yes ) {
            KIO::NetAccess::del( KURL(outputDir), Window::theMainWindow() );
        }
        else
            return false;
    }
    return true;
}

int HTMLExportDialog::calculateSteps()
{
    int count = 0;
    for( QValueList<ImageSizeCheckBox*>::Iterator it2 = _cbs.begin(); it2 != _cbs.end(); ++it2 ) {
        if ( (*it2)->isChecked() )
            count++;
    }

    return _list.count() * ( 1 + count ); // 1 thumbnail + 1 real image
}

QString HTMLExportDialog::namePage( int width, int height, const QString& fileName )
{
    QString name = _nameMap[fileName];
    QString base = QFileInfo( name ).baseName(true);
    return QString::fromLatin1( "%1-%2.html" ).arg( base ).arg( ImageSizeCheckBox::text(width,height,true) );
}

QString HTMLExportDialog::nameImage( const QString& fileName, int size )
{
    QString name = _nameMap[fileName];
    QString base = QFileInfo( name ).baseName(true);
    QString ext = QFileInfo( name ).extension(false);
    if ( size == _maxImageSize )
        return name;
    else
        return QString::fromLatin1( "%1-%2.%3" ).arg( base ).arg( size ).arg( ext );
}


bool HTMLExportDialog::linkIndexFile()
{
    for( QValueList<ImageSizeCheckBox*>::Iterator it = _preferredSizes.begin();
         it != _preferredSizes.end(); ++it ) {
        if ( (*it)->isChecked() ) {
            QString fromFile = QString::fromLatin1("index-%1.html" )
                               .arg((*it)->text(true));
            QString destFile = _tempDir + QString::fromLatin1("/index.html");
            // bool ok = ( symlink( QFile::encodeName(fromFile), QFile::encodeName(destFile) ) == 0 );
            kdDebug() <<fromFile << " " << QFile( fromFile ).exists() << " " << destFile << endl;
            bool ok = Utilities::copy( QFileInfo(destFile).dirPath() + fromFile, destFile );
            if ( !ok ) {
                KMessageBox::error( this, i18n("<qt>Unable to copy %1 to %2</qt>")
                                    .arg( fromFile ).arg( destFile ) );

                return false;
            }
            return ok;
        }
    }
    return false;
}


QValueList<ImageSizeCheckBox*> HTMLExportDialog::activeResolutions()
{
    QValueList<ImageSizeCheckBox*> res;
    for( QValueList<ImageSizeCheckBox*>::Iterator sizeIt = _cbs.begin(); sizeIt != _cbs.end(); ++sizeIt ) {
        if ( (*sizeIt)->isChecked() ) {
            res << *sizeIt;
            _maxImageSize = (*sizeIt)->width();
        }
    }
    return res;
}

void HTMLExportDialog::populateThemesCombo()
{
    QStringList dirs = KGlobal::dirs()->findDirs( "data", QString::fromLocal8Bit("kphotoalbum/themes/") );
    int i = 0;
    for(QStringList::Iterator it = dirs.begin(); it != dirs.end(); ++it) {
        QDir dir(*it);
        QStringList themes = dir.entryList( QDir::Dirs | QDir::Readable );
        for(QStringList::Iterator it = themes.begin(); it != themes.end(); ++it) {
            if(*it == QString::fromLatin1(".") || *it == QString::fromLatin1("..")) continue;
            QString themePath = QString::fromLatin1("%1/%2/").arg(dir.path()).arg(*it);

            KSimpleConfig themeConfig( QString::fromLatin1( "%1kphotoalbum.theme" ).arg( themePath ), true );
            if( !themeConfig.hasGroup( QString::fromLatin1( "theme" ) ) ) {
                kdDebug() << QString::fromLatin1("invalid theme: %1 (missing theme section)").arg( *it )
                          << endl;
                continue;
            }
            themeConfig.setGroup( QString::fromLatin1( "theme" ) );
            QString themeName = themeConfig.readEntry( "Name" );
            QString themeAuthor = themeConfig.readEntry( "Author" );

            enableButtonOK( true );
            _themeBox->insertItem( i18n( "%1 (by %2)" ).arg( themeName ).arg( themeAuthor ), i );
            _themes.insert( i, themePath );
            i++;
        }
    }
    if(_themeBox->count() < 1) {
        KMessageBox::error( this, i18n("Could not find any themes - this is very likely an installation error" ) );
    }
}

void HTMLExportDialog::getThemeInfo( QString* baseDir, QString* name, QString* author )
{
    *baseDir = _themes[_themeBox->currentItem()];
    KSimpleConfig themeConfig( QString::fromLatin1( "%1kphotoalbum.theme" ).arg( *baseDir ), true );
    themeConfig.setGroup( QString::fromLatin1( "theme" ) );
    *name = themeConfig.readEntry( "Name" );
    *author = themeConfig.readEntry( "Author" );
}

int HTMLExportDialog::exec( const QStringList& list )
{
    _list = list;
    return KDialogBase::exec();
}

QString HTMLExportDialog::kimFileName( bool relative )
{
    if ( relative )
        return QString::fromLatin1( "%2.kim" ).arg( _outputDir->text() );
    else
        return QString::fromLatin1( "%1/%2.kim" ).arg( _tempDir ).arg( _outputDir->text() );
}

QString HTMLExportDialog::translateToHTML( const QString& str )
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

#include "HtmlExportDialog.moc"
