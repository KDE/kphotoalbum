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

#include "Import.h"
#include <kfiledialog.h>
#include <qlabel.h>
#include <klocale.h>
#include <qpushbutton.h>
#include <qdom.h>
#include <qfile.h>
#include <kmessagebox.h>
#include <kzip.h>
#include <karchive.h>
#include <qlayout.h>
#include <klineedit.h>
#include <kpushbutton.h>
#include "Settings/SettingsData.h"
#include "ImportMatcher.h"
#include <qcheckbox.h>
#include "Utilities/Util.h"
#include "DB/ImageDB.h"
#include <qimage.h>
#include "Browser/BrowserWidget.h"
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kurl.h>
#include <qprogressdialog.h>
#include <kio/netaccess.h>
#include "MainWindow/Window.h"
#include <kapplication.h>
#include "DB/CategoryCollection.h"
#include "DB/ImageInfo.h"
#include "MiniViewer.h"
#include "XMLDB/Database.h"

class KPushButton;
using namespace ImportExport;

void Import::imageImport()
{
    KURL url = KFileDialog::getOpenURL( QString::null, QString::fromLatin1( "*.kim|KPhotoAlbum Export Files" ), 0 );
    if ( url.isEmpty() )
        return;
    imageImport( url );
}

void Import::imageImport( const KURL& url )
{
    bool ok;
    if ( !url.isLocalFile() ) {
        new Import( url, 0, "import_dialog" );
        // The dialog will start the download, and in the end show itself
    }
    else {
        Import* dialog = new Import( url.path(), &ok, 0, "import_dialog" );
        dialog->resize( 800, 600 );
        if ( ok )
            dialog->show();
        else
            delete dialog;
    }
}

Import::Import( const KURL& url, QWidget* parent, const char* name )
    :KWizard( parent, name, false ), _zip( 0 ), _hasFilled( false )
{
    _kimFile = url;
    _tmp = new KTempFile( QString::null, QString::fromLatin1( ".kim" ) );
    QString path = _tmp->name();
    _tmp->setAutoDelete( true );

    KURL dest;
    dest.setPath( path );
    KIO::FileCopyJob* job = KIO::file_copy( url, dest, -1, true );
    connect( job, SIGNAL( result( KIO::Job* ) ), this, SLOT( downloadKimJobCompleted( KIO::Job* ) ) );
}

void Import::downloadKimJobCompleted( KIO::Job* job )
{
    if ( !job->error() ) {
        resize( 800, 600 );
        init( _tmp->name() );
        show();
    }
    else {
        job->showErrorDialog( 0 );
        delete this;
    }
}

Import::Import( const QString& fileName, bool* ok, QWidget* parent, const char* name )
    :KWizard( parent, name, false ), _zipFile( fileName ), _tmp(0), _hasFilled( false )
{
    _kimFile.setPath( fileName );
    *ok = init( fileName );
}

bool Import::init( const QString& fileName )
{
    _finishedPressed = false;
    _zip = new KZip( fileName );
    if ( !_zip->open( IO_ReadOnly ) ) {
        KMessageBox::error( this, i18n("Unable to open '%1' for reading.").arg( fileName ), i18n("Error Importing Data") );
        _zip =0;
        return false;
    }
    _dir = _zip->directory();
    if ( _dir == 0 ) {
        KMessageBox::error( this, i18n( "Error reading directory contents of file %1; it is likely that the file is broken." ).arg( fileName ) );
        return false;
    }

    const KArchiveEntry* indexxml = _dir->entry( QString::fromLatin1( "index.xml" ) );
    if ( indexxml == 0 || ! indexxml->isFile() ) {
        KMessageBox::error( this, i18n( "Error reading index.xml file from %1; it is likely that the file is broken." ).arg( fileName ) );
        return false;
    }

    const KArchiveFile* file = static_cast<const KArchiveFile*>( indexxml );
    QByteArray data = file->data();

    bool ok = readFile( data, fileName );
    if ( !ok )
        return false;

    setupPages();
    return true;
}

Import::~Import()
{
    delete _zip;
    delete _tmp;
}

bool Import::readFile( const QByteArray& data, const QString& fileName )
{
    QDomDocument doc;
    QString errMsg;
    int errLine;
    int errCol;

    if ( !doc.setContent( data, false, &errMsg, &errLine, &errCol )) {
        KMessageBox::error( this, i18n( "Error in file %1 on line %2 col %3: %4" ).arg(fileName).arg(errLine).arg(errCol).arg(errMsg) );
        return false;
    }

    QDomElement top = doc.documentElement();
    if ( top.tagName().lower() != QString::fromLatin1( "kimdaba-export" ) &&
        top.tagName().lower() != QString::fromLatin1( "kphotoalbum-export" ) ) {
        KMessageBox::error( this, i18n("Unexpected top element while reading file %1. Expected KPhotoAlbum-export found %2")
                            .arg( fileName ).arg( top.tagName() ) );
        return false;
    }

    // Read source
    QString source = top.attribute( QString::fromLatin1( "location" ) ).lower();
    if ( source != QString::fromLatin1( "inline" ) && source != QString::fromLatin1( "external" ) ) {
        KMessageBox::error( this, i18n("<qt>XML file did not specify the source of the images, "
                                       "this is a strong indication that the file is corrupted</qt>" ) );
        return false;
    }

    _externalSource = ( source == QString::fromLatin1( "external" ) );

    // Read base url
    _baseUrl = top.attribute( QString::fromLatin1( "baseurl" ) );

    for ( QDomNode node = top.firstChild(); !node.isNull(); node = node.nextSibling() ) {
        if ( !node.isElement() || ! (node.toElement().tagName().lower() == QString::fromLatin1( "image" ) ) ) {
            KMessageBox::error( this, i18n("Unknown element while reading %1, expected image.").arg( fileName ) );
            return false;
        }
        QDomElement elm = node.toElement();

        DB::ImageInfoPtr info = XMLDB::Database::createImageInfo( elm.attribute( QString::fromLatin1( "file" ) ), elm );
        _images.append( info );
    }

    return true;
}

void Import::setupPages()
{
    createIntroduction();
    createImagesPage();
    createDestination();
    createCategoryPages();
    connect( this, SIGNAL( selected( const QString& ) ), this, SLOT( updateNextButtonState() ) );
    connect( finishButton(), SIGNAL( clicked() ), this, SLOT( slotFinish() ) );
    connect( this, SIGNAL( helpClicked() ), this, SLOT( slotHelp() ) );
}

void Import::createIntroduction()
{
    QString txt = i18n( "<qt><h1><font size=\"+2\">Welcome to KPhotoAlbum Import</font></h1>"
                        "This wizard will take you through the steps of an import operation, The steps are: "
                        "<ul><li>First you must select which images you want to import from the export file. "
                        "You do so by selecting the checkbox next to the image.</li>"
                        "<li>Next you must tell KPhotoAlbum in which directory to put the images. This directory must "
                        "of course be below the directory root KPhotoAlbum uses for images. "
                        "KPhotoAlbum will take care to avoid name clashes</li>"
                        "<li>The next step is to specify which categories you want to import (Persons, Locations, ... ) "
                        "and also tell KPhotoAlbum how to match the categories from the file to your categories. "
                        "Imagine you load from a file, where a category is called <tt>Blomst</tt> (which is the "
                        "Danish word for flower), then you would likely want to match this with your category, which might be "
                        "called <tt>Blume</tt> (which is the German word for flower) - of course given you are German.</li>"
                        "<li>The final steps, is matching the individual tokens from the categories. I may call myself <tt>Jesper</tt> "
                        "in my image database, while you want to call me by my full name, namely <tt>Jesper K. Pedersen</tt>. "
                        "In this step non matches will be highlighted in red, so you can see which tokens was not found in your "
                        "database, or which tokens was only a partial match.</li>"
                        "</p></qt>" );

    QLabel* intro = new QLabel( txt, this );
    addPage( intro, i18n("Introduction") );
}

void Import::createImagesPage()
{
    QScrollView* top = new QScrollView( this );
    top->setResizePolicy( QScrollView::AutoOneFit );

    QWidget* container = new QWidget( this );
    QVBoxLayout* lay1 = new QVBoxLayout( container, 6 );
    top->addChild( container );

    // Select all and Deselect All buttons
    QHBoxLayout* lay2 = new QHBoxLayout( lay1, 6 );
    QPushButton* selectAll = new QPushButton( i18n("Select All"), container );
    lay2->addWidget( selectAll );
    QPushButton* selectNone = new QPushButton( i18n("Deselect All"), container );
    lay2->addWidget( selectNone );
    lay2->addStretch( 1 );
    connect( selectAll, SIGNAL( clicked() ), this, SLOT( slotSelectAll() ) );
    connect( selectNone, SIGNAL( clicked() ), this, SLOT( slotSelectNone() ) );

    QGridLayout* lay3 = new QGridLayout( lay1, _images.count(), 3, 6 );
    lay3->setColStretch( 2, 1 );

    int row = 0;
    for( DB::ImageInfoListConstIterator it = _images.constBegin(); it != _images.constEnd(); ++it, ++row ) {
        DB::ImageInfoPtr info = *it;
        ImageRow* ir = new ImageRow( info, this, container );
        lay3->addWidget( ir->_checkbox, row, 0 );

        const KArchiveEntry* thumbnails = _dir->entry( QString::fromLatin1( "Thumbnails" ) );
        if ( thumbnails ) {
            QPushButton* but = new QPushButton( container, "image" );
            but->setPixmap( loadThumbnail( info->fileName( true ) ) );
            lay3->addWidget( but, row, 1 );
            connect( but, SIGNAL( clicked() ), ir, SLOT( showImage() ) );
        }
        else {
            QLabel* label = new QLabel( info->label(), container, "filename" );
            lay3->addWidget( label, row, 1 );
        }

        QLabel* label = new QLabel( QString::fromLatin1("<qt>%1</qt>").arg(info->description()), container, "description" );
        lay3->addWidget( label, row, 2 );
        _imagesSelect.append( ir );
    }

    addPage( top, i18n("Select Which Images to Import") );
}

ImageRow::ImageRow( DB::ImageInfoPtr info, Import* import, QWidget* parent )
    : QObject( parent ), _info( info ), _import( import )
{
    _checkbox = new QCheckBox( QString::null, parent, "_checkbox" );
    _checkbox->setChecked( true );
}

void ImageRow::showImage()
{
    if ( _import->_externalSource ) {
        KURL src1 =_import->_kimFile;
        KURL src2 = _import->_baseUrl + QString::fromLatin1( "/" );
        for ( int i = 0; i < 2; ++i ) {
            // First try next to the .kim file, then the external URL
            KURL src = src1;
            if ( i == 1 )
                src = src2;
            src.setFileName( _info->fileName( true ) );
            QString tmpFile;

            if( KIO::NetAccess::download( src, tmpFile, MainWindow::Window::theMainWindow() ) ) {
                QImage img( tmpFile );
                MiniViewer::show( img, _info );
                KIO::NetAccess::removeTempFile( tmpFile );
                break;
            }
        }
    }
    else {
        QImage img = QImage( _import->loadImage( _info->fileName(true) ) );
        MiniViewer::show( img, _info );
    }
}

void Import::createDestination()
{
    QWidget* top = new QWidget( this );
    _destinationPage = top;
    QVBoxLayout* topLay = new QVBoxLayout( top, 6 );
    QHBoxLayout* lay = new QHBoxLayout( topLay, 6 );
    topLay->addStretch( 1 );

    QLabel* label = new QLabel( i18n( "Destination of images: " ), top );
    lay->addWidget( label );

    _destinationEdit = new KLineEdit( top );
    lay->addWidget( _destinationEdit, 1 );

    KPushButton* but = new KPushButton( QString::fromLatin1("..." ), top );
    but->setFixedWidth( 30 );
    lay->addWidget( but );


    _destinationEdit->setText( Settings::SettingsData::instance()->imageDirectory());
    connect( but, SIGNAL( clicked() ), this, SLOT( slotEditDestination() ) );
    connect( _destinationEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( updateNextButtonState() ) );
    addPage( top, i18n("Destination of Images" ) );
}

void  Import::slotEditDestination()
{
    QString file = KFileDialog::getExistingDirectory( _destinationEdit->text(), this );
    if ( !file.isNull() ) {
        if ( ! QFileInfo(file).absFilePath().startsWith( QFileInfo(Settings::SettingsData::instance()->imageDirectory()).absFilePath()) ) {
            KMessageBox::error( this, i18n("The directory must be a subdirectory of %1").arg( Settings::SettingsData::instance()->imageDirectory() ) );
        }
        else {
            _destinationEdit->setText( file );
            updateNextButtonState();
        }
    }
}

void Import::updateNextButtonState()
{
    bool enabled = true;
    if ( currentPage() == _destinationPage ) {
        QString dest = _destinationEdit->text();
        if ( QFileInfo( dest ).isFile() )
            enabled = false;
        else if ( ! QFileInfo(dest).absFilePath().startsWith( QFileInfo(Settings::SettingsData::instance()->imageDirectory()).absFilePath()) )
            enabled = false;
    }

    nextButton()->setEnabled( enabled );
}

void Import::createCategoryPages()
{
    QStringList categories;
    DB::ImageInfoList images = selectedImages();
    for( DB::ImageInfoListConstIterator it = images.constBegin(); it != images.constEnd(); ++it ) {
        DB::ImageInfoPtr info = *it;
        QStringList categoriesForImage = info->availableCategories();
        for( QStringList::Iterator categoryIt = categoriesForImage.begin(); categoryIt != categoriesForImage.end(); ++categoryIt ) {
            if ( !categories.contains( *categoryIt ) &&
                 (*categoryIt) != QString::fromLatin1( "Folder" ) &&
                 (*categoryIt) != QString::fromLatin1( "Tokens" ) )
                categories.append( *categoryIt );
        }
    }

    _categoryMatcher = new ImportMatcher( QString::null, QString::null, categories, DB::ImageDB::instance()->categoryCollection()->categoryNames(),
                                          false, this, "import matcher" );
    addPage( _categoryMatcher, i18n("Match Categories") );

    _dummy = new QWidget( this );
    addPage( _dummy, QString::null );
}

ImportMatcher* Import::createCategoryPage( const QString& myCategory, const QString& otherCategory )
{
    QStringList otherItems;
    DB::ImageInfoList images = selectedImages();
    for( DB::ImageInfoListConstIterator it = images.constBegin(); it != images.constEnd(); ++it ) {
        DB::ImageInfoPtr info = *it;
        StringSet items = info->itemsOfCategory( otherCategory );
        for( StringSet::ConstIterator itemIt = items.begin(); itemIt != items.end(); ++itemIt )
            otherItems.append( *itemIt );
    }

    QStringList myItems = DB::ImageDB::instance()->categoryCollection()->categoryForName( myCategory )->itemsInclCategories();
    myItems.sort();

    ImportMatcher* matcher = new ImportMatcher( otherCategory, myCategory, otherItems, myItems, true, this, "import matcher" );
    addPage( matcher, myCategory );
    return matcher;
}

void Import::next()
{
    if ( currentPage() == _destinationPage ) {
        QString dir = _destinationEdit->text();
        if ( !QFileInfo( dir ).exists() ) {
            int answer = KMessageBox::questionYesNo( this, i18n("Directory %1 does not exists. Should it be created?").arg( dir ) );
            if ( answer == KMessageBox::Yes ) {
                bool ok = KStandardDirs::makeDir( dir );
                if ( !ok ) {
                    KMessageBox::error( this, i18n("Error creating directory %1").arg( dir ) );
                    return;
                }
            }
            else
                return;
        }
    }
    if ( !_hasFilled && currentPage() == _categoryMatcher ) {
        _hasFilled = true;
        _categoryMatcher->setEnabled( false );
        delete _dummy;

        ImportMatcher* matcher = 0;
        for( QValueList<CategoryMatch*>::Iterator it = _categoryMatcher->_matchers.begin();
             it != _categoryMatcher->_matchers.end();
             ++it )
        {
            CategoryMatch* match = *it;
            if ( match->_checkbox->isChecked() ) {
                matcher = createCategoryPage( match->_combobox->currentText(), match->_text );
                _matchers.append( matcher );
            }
        }
        if ( matcher )
            setFinishEnabled( matcher, true );
        else
            setFinishEnabled( _categoryMatcher, true );
    }

    QWizard::next();
}

bool Import::copyFilesFromZipFile()
{
    DB::ImageInfoList images = selectedImages();
    for( DB::ImageInfoListConstIterator it = images.constBegin(); it != images.constEnd(); ++it ) {
        QString fileName = (*it)->fileName( true );
        QByteArray data = loadImage( fileName );
        if ( data.isNull() )
            return false;
        QString newName = Settings::SettingsData::instance()->imageDirectory() + _nameMap[fileName];

        QString relativeName = newName.mid( Settings::SettingsData::instance()->imageDirectory().length() );
        if ( relativeName.startsWith( QString::fromLatin1( "/" ) ) )
            relativeName= relativeName.mid(1);

        QFile out( newName );
        if ( !out.open( IO_WriteOnly ) ) {
            KMessageBox::error( this, i18n("Error when writing image %s").arg( newName ) );
            return false;
        }
        out.writeBlock( data, data.size() );
        out.close();
    }
    return true;
}

void Import::copyFromExternal()
{
    _pendingCopies = selectedImages();
    _totalCopied = 0;
    _progress = new QProgressDialog( i18n("Copying Images"), i18n("&Cancel"), _pendingCopies.count(), 0, "_progress", true );
    _progress->setProgress( 0 );
    _progress->show();
    connect( _progress, SIGNAL( canceled() ), this, SLOT( stopCopyingImages() ) );
    copyNextFromExternal();
}

void Import::copyNextFromExternal()
{
    DB::ImageInfoPtr info = _pendingCopies[0];
    _pendingCopies.pop_front();
    QString fileName = info->fileName( true );
    KURL src1 = _kimFile;
    KURL src2 = _baseUrl + QString::fromLatin1( "/" );
    for ( int i = 0; i < 2; ++i ) {
        KURL src = src1;
        if ( i == 1 )
            src = src2;

        src.setFileName( fileName );
        if ( KIO::NetAccess::exists( src, true, MainWindow::Window::theMainWindow() ) ) {
            KURL dest;
            dest.setPath( Settings::SettingsData::instance()->imageDirectory() + _nameMap[fileName] );
            _job = KIO::file_copy( src, dest, -1, false, false, false );
            connect( _job, SIGNAL( result( KIO::Job* ) ), this, SLOT( aCopyJobCompleted( KIO::Job* ) ) );
            break;
        }
    }
}


void Import::aCopyJobCompleted( KIO::Job* job )
{
    if ( job->error() ) {
        job->showErrorDialog( 0 );
        deleteLater();
        delete _progress;
    }
    else if ( _pendingCopies.count() == 0 ) {
        updateDB();
        deleteLater();
        delete _progress;
    }
    else if ( _progress->wasCanceled() ) {
        deleteLater();
        delete _progress;
    }
    else {
        _progress->setProgress( ++_totalCopied );
        copyNextFromExternal();
    }
}

void Import::stopCopyingImages()
{
    _job->kill( true );
}

void Import::slotFinish()
{
    _finishedPressed = true;
    _nameMap = Utilities::createUniqNameMap( Utilities::infoListToStringList(selectedImages()), true, _destinationEdit->text() );
    bool ok;
    if ( _externalSource ) {
        hide();
        copyFromExternal();
    }
    else {
        ok = copyFilesFromZipFile();
        if ( ok )
            updateDB();
        deleteLater();
    }
}

void Import::updateDB()
{
    // Run though all images
    DB::ImageInfoList images = selectedImages();
    for( DB::ImageInfoListConstIterator it = images.constBegin(); it != images.constEnd(); ++it ) {
        DB::ImageInfoPtr info = *it;

        DB::ImageInfoPtr newInfo = new DB::ImageInfo( _nameMap[info->fileName(true)] );
        newInfo->setLabel( info->label() );
        newInfo->setDescription( info->description() );
        newInfo->setDate( info->date() );
        newInfo->rotate( info->angle() );
        newInfo->setDrawList( info->drawList() );
        newInfo->setMD5Sum( info->MD5Sum() );
        DB::ImageInfoList list;
        list.append(newInfo);
        DB::ImageDB::instance()->addImages( list );

        // Run though the categories
        for( QValueList<ImportMatcher*>::Iterator grpIt = _matchers.begin(); grpIt != _matchers.end(); ++grpIt ) {
            QString otherGrp = (*grpIt)->_otherCategory;
            QString myGrp = (*grpIt)->_myCategory;

            // Run through each option
            QValueList<CategoryMatch*>& matcher = (*grpIt)->_matchers;
            for( QValueList<CategoryMatch*>::Iterator optionIt = matcher.begin(); optionIt != matcher.end(); ++optionIt ) {
                if ( !(*optionIt)->_checkbox->isChecked() )
                    continue;
                QString otherOption = (*optionIt)->_text;
                QString myOption = (*optionIt)->_combobox->currentText();

                if ( info->hasCategoryInfo( otherGrp, otherOption ) ) {
                    newInfo->addCategoryInfo( myGrp, myOption );
                    DB::ImageDB::instance()->categoryCollection()->categoryForName( myGrp )->addItem( myOption );
                }

            }
        }
    }
    Browser::BrowserWidget::instance()->home();
}

QPixmap Import::loadThumbnail( QString fileName )
{
    const KArchiveEntry* thumbnails = _dir->entry( QString::fromLatin1( "Thumbnails" ) );
    Q_ASSERT( thumbnails ); // We already tested for this.

    if ( !thumbnails->isDirectory() ) {
        KMessageBox::error( this, i18n("Thumbnail item in export file was not a directory, this indicates that the file is broken.") );
        return QPixmap();
    }

    const KArchiveDirectory* thumbnailDir = static_cast<const KArchiveDirectory*>( thumbnails );

    const QString ext = Utilities::isVideo( fileName ) ? QString::fromLatin1( "jpg" ) : QFileInfo( fileName ).extension();
    fileName = QString::fromLatin1("%1.%2").arg( Utilities::stripSlash( QFileInfo( fileName ).baseName() ) ).arg(ext);
    const KArchiveEntry* fileEntry = thumbnailDir->entry( fileName );
    if ( fileEntry == 0 || !fileEntry->isFile() ) {
        KMessageBox::error( this, i18n("No thumbnail existed in export file for %1").arg( fileName ) );
        return QPixmap();
    }

    const KArchiveFile* file = static_cast<const KArchiveFile*>( fileEntry );
    QByteArray data = file->data();
    return QPixmap( data );
}

void Import::slotSelectAll()
{
    selectImage( true );
}

void Import::slotSelectNone()
{
    selectImage( false );
}

void Import::selectImage( bool on )
{
    for( QValueList<ImageRow*>::Iterator it = _imagesSelect.begin(); it != _imagesSelect.end(); ++it ) {
        (*it)->_checkbox->setChecked( on );
    }
}

QByteArray Import::loadImage( const QString& fileName )
{
    const KArchiveEntry* images = _dir->entry( QString::fromLatin1( "Images" ) );
    if ( !images ) {
        KMessageBox::error( this, i18n("export file did not contain a Images subdirectory, this indicates that the file is broken") );
        return QByteArray();
    }

    if ( !images->isDirectory() ) {
        KMessageBox::error( this, i18n("Images item in export file was not a directory, this indicates that the file is broken") );
        return QByteArray();
    }

    const KArchiveDirectory* imagesDir = static_cast<const KArchiveDirectory*>( images );

    const KArchiveEntry* fileEntry = imagesDir->entry( fileName );
    if ( fileEntry == 0 || !fileEntry->isFile() ) {
        KMessageBox::error( this, i18n("No image existed in export file for %1").arg( fileName ) );
        return QByteArray();
    }

    const KArchiveFile* file = static_cast<const KArchiveFile*>( fileEntry );
    QByteArray data = file->data();
    return data;
}

DB::ImageInfoList Import::selectedImages()
{
    DB::ImageInfoList res;
    for( QValueList<ImageRow*>::Iterator it = _imagesSelect.begin(); it != _imagesSelect.end(); ++it ) {
        if ( (*it)->_checkbox->isChecked() )
            res.append( (*it)->_info );
    }
    return res;
}

void Import::closeEvent( QCloseEvent* e )
{
    // If the user presses the finish button, then we have to postpone the delete operations, as we have pending copies.
    if ( !_finishedPressed )
        deleteLater();
    KWizard::closeEvent( e );
}



void Import::slotHelp()
{
    kapp->invokeHelp( QString::fromLatin1( "kphotoalbum#chp-exportDialog" ) );
}

#include "Import.moc"
