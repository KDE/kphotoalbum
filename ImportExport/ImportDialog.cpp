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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "ImportDialog.h"
#include "ImageRow.h"
#include <kfiledialog.h>
#include <qlabel.h>
#include <QHBoxLayout>
#include <Q3ValueList>
#include <QGridLayout>
#include <QPixmap>
#include <QVBoxLayout>
#include <QCloseEvent>
#include <QProgressDialog>
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
#include <kurl.h>
#include <q3progressdialog.h>
#include <kio/netaccess.h>
#include <kapplication.h>
#include <ktoolinvocation.h>
#include "DB/CategoryCollection.h"
#include "DB/ImageInfo.h"
#include "MiniViewer.h"
#include "XMLDB/Database.h"
#include <kdebug.h>
#include <QComboBox>
#include <QScrollArea>
#include <KMessageBox>

using Utilities::StringSet;

class KPushButton;
using namespace ImportExport;


ImportDialog::ImportDialog( QWidget* parent )
    :KAssistantDialog( parent ), _hasFilled( false )
{
}

bool ImportDialog::exec( const QString& fileName, const KUrl& kimFileURL )
{
    _kimFile = kimFileURL;

#ifdef KDAB_TEMPORARILY_REMOVED
    connect( this, SIGNAL( failedToCopy( QStringList ) ), &m_importHandler, SLOT( aCopyFailed( QStringList ) ) ); // JKP
#else // KDAB_TEMPORARILY_REMOVED
    qWarning("Code commented out in ImportDialog::ImportDialog");
#endif //KDAB_TEMPORARILY_REMOVED


    _zip = new KZip( fileName );
    if ( !_zip->open( QIODevice::ReadOnly ) ) {
        KMessageBox::error( this, i18n("Unable to open '%1' for reading.", fileName ), i18n("Error Importing Data") );
        _zip =0;
        return false;
    }
    _dir = _zip->directory();
    if ( _dir == 0 ) {
        KMessageBox::error( this, i18n( "Error reading directory contents of file %1; it is likely that the file is broken." , fileName ) );
        return false;
    }

    const KArchiveEntry* indexxml = _dir->entry( QString::fromLatin1( "index.xml" ) );
    if ( indexxml == 0 || ! indexxml->isFile() ) {
        KMessageBox::error( this, i18n( "Error reading index.xml file from %1; it is likely that the file is broken." , fileName ) );
        return false;
    }

    const KArchiveFile* file = static_cast<const KArchiveFile*>( indexxml );
    QByteArray data = file->data();

    bool ok = readFile( data, fileName );
    if ( !ok )
        return false;

    setupPages();

    return KAssistantDialog::exec() ;
}

ImportDialog::~ImportDialog()
{
    delete _zip;
}

bool ImportDialog::readFile( const QByteArray& data, const QString& fileName )
{
    QDomDocument doc;
    QString errMsg;
    int errLine;
    int errCol;

    if ( !doc.setContent( data, false, &errMsg, &errLine, &errCol )) {
        KMessageBox::error( this, i18n( "Error in file %1 on line %2 col %3: %4" ,fileName,errLine,errCol,errMsg) );
        return false;
    }

    QDomElement top = doc.documentElement();
    if ( top.tagName().toLower() != QString::fromLatin1( "kimdaba-export" ) &&
        top.tagName().toLower() != QString::fromLatin1( "kphotoalbum-export" ) ) {
        KMessageBox::error( this, i18n("Unexpected top element while reading file %1. Expected KPhotoAlbum-export found %2",
                            fileName ,top.tagName() ) );
        return false;
    }

    // Read source
    QString source = top.attribute( QString::fromLatin1( "location" ) ).toLower();
    if ( source != QString::fromLatin1( "inline" ) && source != QString::fromLatin1( "external" ) ) {
        KMessageBox::error( this, i18n("<p>XML file did not specify the source of the images, "
                                       "this is a strong indication that the file is corrupted</p>" ) );
        return false;
    }

    _externalSource = ( source == QString::fromLatin1( "external" ) );

    // Read base url
    _baseUrl = top.attribute( QString::fromLatin1( "baseurl" ) );

    for ( QDomNode node = top.firstChild(); !node.isNull(); node = node.nextSibling() ) {
        if ( !node.isElement() || ! (node.toElement().tagName().toLower() == QString::fromLatin1( "image" ) ) ) {
            KMessageBox::error( this, i18n("Unknown element while reading %1, expected image.", fileName ) );
            return false;
        }
        QDomElement elm = node.toElement();

        DB::ImageInfoPtr info = XMLDB::Database::createImageInfo( elm.attribute( QString::fromLatin1( "file" ) ), elm );
        _images.append( info );
    }

    return true;
}

void ImportDialog::setupPages()
{
    createIntroduction();
    createImagesPage();
    createDestination();
    createCategoryPages();
    connect( this, SIGNAL( currentPageChanged( KPageWidgetItem*, KPageWidgetItem* ) ), this, SLOT( updateNextButtonState() ) );
#ifdef KDAB_TEMPORARILY_REMOVED
    connect( this, SIGNAL( user1Clicked() ), this, SLOT( slotFinish() ) );
#else // KDAB_TEMPORARILY_REMOVED
    qWarning("Code commented out in ImportDialog::setupPages");
#endif //KDAB_TEMPORARILY_REMOVED
    connect( this, SIGNAL( helpClicked() ), this, SLOT( slotHelp() ) );
}

void ImportDialog::createIntroduction()
{
    QString txt = i18n( "<h1><font size=\"+2\">Welcome to KPhotoAlbum Import</font></h1>"
                        "This wizard will take you through the steps of an import operation. The steps are: "
                        "<ol><li>First you must select which images you want to import from the export file. "
                        "You do so by selecting the checkbox next to the image.</li>"
                        "<li>Next you must tell KPhotoAlbum in which directory to put the images. This directory must "
                        "of course be below the directory root KPhotoAlbum uses for images. "
                        "KPhotoAlbum will take care to avoid name clashes</li>"
                        "<li>The next step is to specify which categories you want to import (People, Places, ... ) "
                        "and also tell KPhotoAlbum how to match the categories from the file to your categories. "
                        "Imagine you load from a file, where a category is called <b>Blomst</b> (which is the "
                        "Danish word for flower), then you would likely want to match this with your category, which might be "
                        "called <b>Blume</b> (which is the German word for flower) - of course given you are German.</li>"
                        "<li>The final steps, is matching the individual tokens from the categories. I may call myself <b>Jesper</b> "
                        "in my image database, while you want to call me by my full name, namely <b>Jesper K. Pedersen</b>. "
                        "In this step non matches will be highlighted in red, so you can see which tokens was not found in your "
                        "database, or which tokens was only a partial match.</li></ol>");

    QLabel* intro = new QLabel( txt, this );
    intro->setWordWrap(true);
    addPage( intro, i18n("Introduction") );
}

void ImportDialog::createImagesPage()
{
    QScrollArea* top = new QScrollArea;
    top->setWidgetResizable(true);

    QWidget* container = new QWidget;
    QVBoxLayout* lay1 = new QVBoxLayout( container );
    top->setWidget( container );

    // Select all and Deselect All buttons
    QHBoxLayout* lay2 = new QHBoxLayout;
    lay1->addLayout(lay2);

    QPushButton* selectAll = new QPushButton( i18n("Select All"), container );
    lay2->addWidget( selectAll );
    QPushButton* selectNone = new QPushButton( i18n("Deselect All"), container );
    lay2->addWidget( selectNone );
    lay2->addStretch( 1 );
    connect( selectAll, SIGNAL( clicked() ), this, SLOT( slotSelectAll() ) );
    connect( selectNone, SIGNAL( clicked() ), this, SLOT( slotSelectNone() ) );

    QGridLayout* lay3 = new QGridLayout;
    lay1->addLayout( lay3 );

    lay3->setColumnStretch( 2, 1 );

    int row = 0;
    for( DB::ImageInfoListConstIterator it = _images.constBegin(); it != _images.constEnd(); ++it, ++row ) {
        DB::ImageInfoPtr info = *it;
        ImageRow* ir = new ImageRow( info, this, container );
        lay3->addWidget( ir->_checkbox, row, 0 );

        const KArchiveEntry* thumbnails = _dir->entry( QString::fromLatin1( "Thumbnails" ) );
        if ( thumbnails ) {
            QPushButton* but = new QPushButton( container );
            const QPixmap pixmap = loadThumbnail( info->fileName( true ) );
            but->setIcon( pixmap );
            but->setIconSize( pixmap.size() );
            lay3->addWidget( but, row, 1 );
            connect( but, SIGNAL( clicked() ), ir, SLOT( showImage() ) );
        }
        else {
            QLabel* label = new QLabel( info->label() );
            lay3->addWidget( label, row, 1 );
        }

        QLabel* label = new QLabel( QString::fromLatin1("<p>%1</p>").arg(info->description()) );
        lay3->addWidget( label, row, 2 );
        _imagesSelect.append( ir );
    }

    addPage( top, i18n("Select Which Images to Import") );
}

void ImportDialog::createDestination()
{
    QWidget* top = new QWidget( this );
    QVBoxLayout* topLay = new QVBoxLayout( top );
    QHBoxLayout* lay = new QHBoxLayout;
    topLay->addLayout(lay);

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
    _destinationPage = addPage( top, i18n("Destination of Images" ) );
}

void  ImportDialog::slotEditDestination()
{
    QString file = KFileDialog::getExistingDirectory( _destinationEdit->text(), this );
    if ( !file.isNull() ) {
        if ( ! QFileInfo(file).absoluteFilePath().startsWith( QFileInfo(Settings::SettingsData::instance()->imageDirectory()).absoluteFilePath()) ) {
            KMessageBox::error( this, i18n("The directory must be a subdirectory of %1", Settings::SettingsData::instance()->imageDirectory() ) );
        }
        else {
            _destinationEdit->setText( file );
            updateNextButtonState();
        }
    }
}

void ImportDialog::updateNextButtonState()
{
    bool enabled = true;
    if ( currentPage() == _destinationPage ) {
        QString dest = _destinationEdit->text();
        if ( QFileInfo( dest ).isFile() )
            enabled = false;
        else if ( ! QFileInfo(dest).absoluteFilePath().startsWith( QFileInfo(Settings::SettingsData::instance()->imageDirectory()).absoluteFilePath()) )
            enabled = false;
    }

    setValid( currentPage(), enabled );
}

void ImportDialog::createCategoryPages()
{
    QStringList categories;
    DB::ImageInfoList images = selectedImages();
    for( DB::ImageInfoListConstIterator it = images.constBegin(); it != images.constEnd(); ++it ) {
        DB::ImageInfoPtr info = *it;
        QStringList categoriesForImage = info->availableCategories();
        for( QStringList::Iterator categoryIt = categoriesForImage.begin(); categoryIt != categoriesForImage.end(); ++categoryIt ) {
            if ( !categories.contains( *categoryIt ) &&
                 (*categoryIt) != QString::fromLatin1( "Folder" ) &&
                 (*categoryIt) != QString::fromLatin1( "Tokens" ) &&
                 (*categoryIt) != QString::fromLatin1( "Media Type" ))
                categories.append( *categoryIt );
        }
    }

    if ( !categories.isEmpty() ) {
        _categoryMatcher = new ImportMatcher( QString::null, QString::null, categories, DB::ImageDB::instance()->categoryCollection()->categoryNames(),
                                              false, this, "import matcher" );
        _categoryMatcherPage = addPage( _categoryMatcher, i18n("Match Categories") );

        QWidget* dummy = new QWidget;
        _dummy = addPage( dummy, QString::null );
    }
    else
        _categoryMatcherPage = 0;
}

ImportMatcher* ImportDialog::createCategoryPage( const QString& myCategory, const QString& otherCategory )
{
    StringSet otherItems;
    DB::ImageInfoList images = selectedImages();
    for( DB::ImageInfoListConstIterator it = images.constBegin(); it != images.constEnd(); ++it ) {
        otherItems += (*it)->itemsOfCategory( otherCategory );
    }

    QStringList myItems = DB::ImageDB::instance()->categoryCollection()->categoryForName( myCategory )->itemsInclCategories();
    myItems.sort();

    ImportMatcher* matcher = new ImportMatcher( otherCategory, myCategory, otherItems.toList(), myItems, true, this, "import matcher" );
    addPage( matcher, myCategory );
    return matcher;
}

void ImportDialog::next()
{
    if ( currentPage() == _destinationPage ) {
        QString dir = _destinationEdit->text();
        if ( !QFileInfo( dir ).exists() ) {
            int answer = KMessageBox::questionYesNo( this, i18n("Directory %1 does not exists. Should it be created?", dir ) );
            if ( answer == KMessageBox::Yes ) {
                bool ok = KStandardDirs::makeDir( dir );
                if ( !ok ) {
                    KMessageBox::error( this, i18n("Error creating directory %1", dir ) );
                    return;
                }
            }
            else
                return;
        }
    }
    if ( !_hasFilled && currentPage() == _categoryMatcherPage ) {
        _hasFilled = true;
        _categoryMatcher->setEnabled( false );
        removePage(_dummy);

        ImportMatcher* matcher = 0;
        for( QList<CategoryMatch*>::Iterator it = _categoryMatcher->_matchers.begin();
             it != _categoryMatcher->_matchers.end();
             ++it )
        {
            CategoryMatch* match = *it;
            if ( match->_checkbox->isChecked() ) {
                matcher = createCategoryPage( match->_combobox->currentText(), match->_text );
                _matchers.append( matcher );
            }
        }
    }

    KAssistantDialog::next();
}


QPixmap ImportDialog::loadThumbnail( QString fileName )
{
    const KArchiveEntry* thumbnails = _dir->entry( QString::fromLatin1( "Thumbnails" ) );
    Q_ASSERT( thumbnails ); // We already tested for this.

    if ( !thumbnails->isDirectory() ) {
        KMessageBox::error( this, i18n("Thumbnail item in export file was not a directory, this indicates that the file is broken.") );
        return QPixmap();
    }

    const KArchiveDirectory* thumbnailDir = static_cast<const KArchiveDirectory*>( thumbnails );

    const QString ext = Utilities::isVideo( fileName ) ? QString::fromLatin1( "jpg" ) : QFileInfo( fileName ).completeSuffix();
    fileName = QString::fromLatin1("%1.%2").arg( Utilities::stripSlash( QFileInfo( fileName ).baseName() ) ).arg(ext);
    const KArchiveEntry* fileEntry = thumbnailDir->entry( fileName );
    if ( fileEntry == 0 || !fileEntry->isFile() ) {
        KMessageBox::error( this, i18n("No thumbnail existed in export file for %1", fileName ) );
        return QPixmap();
    }

    const KArchiveFile* file = static_cast<const KArchiveFile*>( fileEntry );
    QByteArray data = file->data();
    QPixmap pixmap;
    pixmap.loadFromData( data );
    return pixmap;
}

void ImportDialog::slotSelectAll()
{
    selectImage( true );
}

void ImportDialog::slotSelectNone()
{
    selectImage( false );
}

void ImportDialog::selectImage( bool on )
{
    for( QList<ImageRow*>::Iterator it = _imagesSelect.begin(); it != _imagesSelect.end(); ++it ) {
        (*it)->_checkbox->setChecked( on );
    }
}

QByteArray ImportDialog::loadImage( const QString& fileName )
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
        KMessageBox::error( this, i18n("No image existed in export file for %1", fileName ) );
        return QByteArray();
    }

    const KArchiveFile* file = static_cast<const KArchiveFile*>( fileEntry );
    QByteArray data = file->data();
    return data;
}

DB::ImageInfoList ImportDialog::selectedImages()
{
    DB::ImageInfoList res;
    for( QList<ImageRow*>::Iterator it = _imagesSelect.begin(); it != _imagesSelect.end(); ++it ) {
        if ( (*it)->_checkbox->isChecked() )
            res.append( (*it)->_info );
    }
    return res;
}

void ImportDialog::closeEvent( QCloseEvent* e )
{
#ifdef KDAB_TEMPORARILY_REMOVED
    // If the user presses the finish button, then we have to postpone the delete operations, as we have pending copies.
    if ( !m_importHandler.m_finishedPressed )
        deleteLater();
    KAssistantDialog::closeEvent( e );
#else // KDAB_TEMPORARILY_REMOVED
    qWarning("Code commented out in ImportDialog::closeEvent");
#endif //KDAB_TEMPORARILY_REMOVED
}



void ImportDialog::slotHelp()
{
    KToolInvocation::invokeHelp( QString::fromLatin1( "kphotoalbum#chp-exportDialog" ) );
}

#include "ImportDialog.moc"
