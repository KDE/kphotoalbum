/*
 *  Copyright (c) 2003 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "kdeversion.h"
#include "htmlexportdialog.h"
#include <klocale.h>
#include <qlayout.h>
#include <klineedit.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qdir.h>
#include <qfile.h>
#include <qapplication.h>
#include <qeventloop.h>
#include "imagemanager.h"
#include <qcheckbox.h>
#include <kfiledialog.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include "options.h"
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

class MyCheckBox :public QCheckBox {

public:
    MyCheckBox( int width, int height, QWidget* parent )
        :QCheckBox( QString::fromLatin1("%1x%2").arg(width).arg(height), parent ),
         _width( width ), _height( height )
        {
        }

    MyCheckBox( const QString& text, QWidget* parent )
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


HTMLExportDialog::HTMLExportDialog( const ImageInfoList& list, QWidget* parent, const char* name )
    :KDialogBase( Plain, i18n("HTML Export"), Ok|Cancel, Ok, parent, name ), _list(list)
{
    QWidget* generalPage = plainPage();
    QVBoxLayout* lay1 = new QVBoxLayout( generalPage, 6 );
    QGridLayout* lay2 = new QGridLayout( lay1, 2 );

    QLabel* label = new QLabel( i18n("Page title:"), generalPage );
    lay2->addWidget( label, 0, 0 );
    _title = new KLineEdit( generalPage );
    lay2->addWidget( _title, 0, 1 );

    // Description
    label = new QLabel( i18n("Description:"), generalPage );
    lay2->addWidget( label, 1, 0 );
    _description = new QTextEdit( generalPage );
    lay2->addWidget( _description, 1, 1 );

    // Thumbnail size
    label = new QLabel( i18n("Thumbnail size:"), generalPage );
    lay2->addWidget( label, 2, 0 );

    QHBoxLayout* lay3 = new QHBoxLayout( 0 );
    lay2->addLayout( lay3, 2, 1 );

    _thumbSize = new QSpinBox( 16, 256, 1, generalPage );
    _thumbSize->setValue( 128 );
    lay3->addWidget( _thumbSize );
    lay3->addStretch(1);

    // Number of columns
    label = new QLabel( i18n("Number of columns:"), generalPage );
    lay2->addWidget( label, 3, 0 );

    QHBoxLayout* lay4 = new QHBoxLayout( (QWidget*)0, 0, 6 );
    lay2->addLayout( lay4, 3, 1 );

    QSpinBox* number = new QSpinBox( 1, 10, 1, generalPage );
    lay4->addWidget( number );

    _numOfCols = new QSlider( 1, 10, 1, 5, Horizontal, generalPage );
    _numOfCols->setTickmarks( QSlider::Below );
    lay4->addWidget( _numOfCols );

    connect( _numOfCols, SIGNAL( valueChanged( int ) ), number, SLOT( setValue(int) ) );
    connect( number, SIGNAL( valueChanged( int ) ), _numOfCols, SLOT( setValue(int) ) );
    number->setValue( _numOfCols->value() );

    // Seperator
    QFrame* sep = new QFrame( generalPage );
    sep->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    lay2->addMultiCellWidget( sep, 4, 4, 0, 1 );

    // Base Directory
    label = new QLabel( i18n("Base directory:"), generalPage );
    lay2->addWidget( label, 5, 0 );

    QHBoxLayout* lay5 = new QHBoxLayout( (QWidget*)0, 0, 6 );
    lay2->addLayout( lay5, 5, 1 );

    _baseDir = new KLineEdit( generalPage );
    lay5->addWidget( _baseDir );

    QPushButton* but = new QPushButton( QString::fromLatin1( ".." ), generalPage );
    lay5->addWidget( but );
    but->setFixedWidth( 25 );

    connect( but, SIGNAL( clicked() ), this, SLOT( selectDir() ) );
    _baseDir->setText( Options::instance()->HTMLBaseDir() );

    // Base URL
    label = new QLabel( i18n("Base URL:"), generalPage );
    lay2->addWidget( label, 6, 0 );

    _baseURL = new KLineEdit( generalPage );
    _baseURL->setText( Options::instance()->HTMLBaseURL() );
    lay2->addWidget( _baseURL, 6, 1 );

    // Output Directory
    label = new QLabel( i18n("Output directory:"), generalPage );
    lay2->addWidget( label, 7, 0 );
    _outputDir = new KLineEdit( generalPage );
    lay2->addWidget( _outputDir, 7, 1 );

    // Sponsoring link
    _sponsoringLink = new QCheckBox( i18n("Include sponsoring link"), generalPage );
    _sponsoringLink->setChecked( true );
    lay1->addWidget( _sponsoringLink );

    // Image sizes
    QHGroupBox* sizes = new QHGroupBox( i18n("Image Sizes"), generalPage );
    lay1->addWidget( sizes );
    QWidget* content = new QWidget( sizes );
    QGridLayout* lay6 = new QGridLayout( content, 2, 4 );
    lay6->setAutoAdd( true );
    MyCheckBox* size320  = new MyCheckBox( 320, 200, content );
    MyCheckBox* size640  = new MyCheckBox( 640, 480, content );
    MyCheckBox* size800  = new MyCheckBox( 800, 600, content );
    MyCheckBox* size1024 = new MyCheckBox( 1024, 768, content );
    MyCheckBox* size1280 = new MyCheckBox( 1280, 1024, content );
    MyCheckBox* size1600 = new MyCheckBox( 1600, 1200, content );
    MyCheckBox* sizeOrig = new MyCheckBox( i18n("Full size"), content );

    _cbs << size320 << size640 << size800 << size1024 << size1280 << size1600 << sizeOrig;
    _preferredSizes << size800 << size1024 << size1280 << size640 << size1600 << size320 << sizeOrig;

    resize( 500, sizeHint().height() );
}

bool HTMLExportDialog::generate()
{
    if ( !checkVars() )
        return false;

    // prepare the progress dialog
    _total = _waitCounter = calculateSteps();
    _progress->setTotalSteps( _total );
    _progress->setProgress( 0 );
    connect( _progress, SIGNAL( cancelled() ), this, SLOT( slotCancelGenerate() ) );

    _tempDir = KTempDir().name();


    // Gotta be after the creation of _progress.
    if ( _total == 0 ) {
        KMessageBox::error( this, i18n("No image sizes were selected. Please select at least one."), i18n("No Image Sizes Selected") );
        return false;
    }

    hide();

    // Itertate over each of the image sizes needed.
    for( QValueList<MyCheckBox*>::Iterator sizeIt = _cbs.begin(); sizeIt != _cbs.end(); ++sizeIt ) {
        if ( (*sizeIt)->isChecked() ) {
            bool ok = generateIndexPage( (*sizeIt)->width(), (*sizeIt)->height() );
            if ( !ok )
                return false;
            for ( uint index = 0; index < _list.count(); ++index ) {
                ImageInfo* info = _list.at(index);
                ImageInfo* prev = 0;
                ImageInfo* next = 0;
                if ( index != 0 )
                    prev = _list.at(index-1);
                if ( index != _list.count() -1 )
                    next = _list.at(index+1);
                ok = generateContextPage( (*sizeIt)->width(), (*sizeIt)->height(), prev, info, next );
                if (!ok)
                    return false;
            }
        }
    }

    // Now generate the thumbnail images
    for( ImageInfoListIterator it( _list ); *it; ++it ) {
        if ( _progress->wasCancelled() ) {
            return false;
        }
        createImage( *it, _thumbSize->value() );
    }


    bool ok = linkIndexFile();
    if ( !ok )
        return false;

    if ( _waitCounter > 0 )
        qApp->eventLoop()->enterLoop();

    // Copy files over to destination.
    QString outputDir = _baseDir->text() + QString::fromLatin1( "/" ) + _outputDir->text();
    KIO::CopyJob* job = KIO::move( _tempDir, outputDir );
    connect( job, SIGNAL( result( KIO::Job* ) ), this, SLOT( showBrowser() ) );

    return true;
}

bool HTMLExportDialog::generateIndexPage( int width, int height )
{
    // -------------------------------------------------- Header information + title + description
    QDomDocument doc;
    QDomElement elm;
    QDomElement col;
    QDomElement body = createHTMLHeader( doc, _title->text() );

    if ( !_title->text().isEmpty() ) {
        elm = doc.createElement( QString::fromLatin1( "h1" ) );
        body.appendChild( elm );
        elm.appendChild( doc.createTextNode( _title->text() ) );
    }

    // Description of the page, as this must be HTML code from the user we will backpatch it in later.
    body.appendChild( doc.createTextNode( QString::fromLatin1("###DESCRIPTION###") ) );

    // -------------------------------------------------- Thumbnails
    QDomElement table = doc.createElement( QString::fromLatin1( "table" ) );
    table.setAttribute( QString::fromLatin1( "width" ), QString::fromLatin1( "100%" ) );
    body.appendChild( table );

    int count = 0;
    int cols = _numOfCols->value();
    QDomElement row;
    for( ImageInfoListIterator it( _list ); *it; ++it ) {
        if ( _progress->wasCancelled() ) {
            return false;
        }

        if ( count % cols == 0 ) {
            row = doc.createElement( QString::fromLatin1( "tr" ) );
            row.setAttribute( QString::fromLatin1( "valign" ), QString::fromLatin1( "bottom" ) );
            row.setAttribute( QString::fromLatin1( "align" ), QString::fromLatin1( "center" ) );
            table.appendChild( row );
            count = 0;
        }

        col = doc.createElement( QString::fromLatin1( "td" ) );
        row.appendChild( col );

        QDomElement href = doc.createElement( QString::fromLatin1( "a" ) );
        href.setAttribute( QString::fromLatin1( "href" ),
                           namePage( width, height, (*it)->fileName(false) ) );
        col.appendChild( href );

        QDomElement img = doc.createElement( QString::fromLatin1( "img" ) );
        img.setAttribute( QString::fromLatin1( "src" ),
                          nameThumbNail( *it, _thumbSize->value() ) );
        href.appendChild( img );
        ++count;
    }

    table = doc.createElement( QString::fromLatin1( "table" ) );
    table.setAttribute( QString::fromLatin1( "width" ), QString::fromLatin1( "100%" ) );

    body.appendChild ( table );
    row = doc.createElement( QString::fromLatin1( "tr" ) );
    table.appendChild( row );
    // -------------------------------------------------- Resolution
    QValueList<MyCheckBox*> actRes = activeResolutions();
    if ( actRes.count() > 1 ) {
        col = doc.createElement( QString::fromLatin1( "td" ) );
        row.appendChild( col );
        col.appendChild( doc.createTextNode( QString::fromLatin1( "Resolutions: " ) ) );

        for( QValueList<MyCheckBox*>::Iterator sizeIt = actRes.begin();
             sizeIt != actRes.end(); ++sizeIt ) {
            int w = (*sizeIt)->width();
            int h = (*sizeIt)->height();
            QString page = QString::fromLatin1( "index-%1.html" )
                           .arg( MyCheckBox::text( w, h, true ) );
            QString text = (*sizeIt)->text(false);

            if ( width == w && height == h ) {
                col.appendChild( doc.createTextNode( text ) );
            }
            else {
                QDomElement link = createLink( doc, page, text );
                col.appendChild( link );
            }
            col.appendChild( doc.createTextNode( QString::fromLatin1( " " ) ) );
        }
    }

    // -------------------------------------------------- Logo
    col = doc.createElement( QString::fromLatin1( "td" ) );
    col.setAttribute( QString::fromLatin1( "align" ), QString::fromLatin1( "right" ) );
    row.appendChild( col );

    col.appendChild( doc.createTextNode( QString::fromLatin1( "Created by " ) ) );
    QDomElement link = createLink( doc, QString::fromLatin1( "http://ktown.kde.org/kimdaba/" ),
                                   QString::fromLatin1( "KimDaBa" ) );
    col.appendChild( link );



    if ( _progress->wasCancelled() )
        return false;

    // -------------------------------------------------- Sponsoring links
    if ( _sponsoringLink->isChecked() )
        createSponsorLink( doc, body);

    // -------------------------------------------------- write to file
    QString str = doc.toString();
    str.replace( QString::fromLatin1( "###DESCRIPTION###" ), _description->text() );
    QString fileName = _tempDir + QString::fromLatin1("/index-%1.html" )
                       .arg(MyCheckBox::text(width,height,true));
    bool ok = writeToFile( fileName, str );
    if ( !ok )
        return false;

    return true;
}

bool HTMLExportDialog::generateContextPage( int width, int height, ImageInfo* prevInfo,
                                            ImageInfo* info, ImageInfo* nextInfo )
{
    QDomDocument doc;
    QDomElement body = createHTMLHeader( doc, info->label() );

    // Prepare page header
    QDomElement table = doc.createElement( QString::fromLatin1( "table" ) );
    body.appendChild( table );
    QDomElement headerRow = doc.createElement( QString::fromLatin1( "tr" ) );
    table.appendChild( headerRow );

    // -------------------------------------------------- Links
    QDomElement links = doc.createElement( QString::fromLatin1( "td" ) );
    links.setAttribute( QString::fromLatin1( "align" ), QString::fromLatin1( "left" ) );
    headerRow.appendChild( links );

    // prev Link
    links.appendChild( doc.createTextNode( QString::fromLatin1( "< " ) ) );
    if ( prevInfo ) {
        QDomElement link = createLink( doc, namePage( width, height, prevInfo->fileName( false ) ),
                                       QString::fromLatin1( "prev" ) );
        links.appendChild( link );
    }
    else
        links.appendChild( doc.createTextNode( QString::fromLatin1( "prev" ) ) );

    links.appendChild( doc.createTextNode( QString::fromLatin1( " | " ) ) );

    // Index Link
    QString indexFile = QString::fromLatin1( "index-%1.html" ).arg(MyCheckBox::text(width,height,true));
    QDomElement indexLink = createLink( doc, indexFile, QString::fromLatin1( "index" ) );
    links.appendChild( indexLink );

    links.appendChild( doc.createTextNode( QString::fromLatin1( " | " ) ) );

    // Next Link
    if ( nextInfo ) {
        QDomElement link = createLink( doc, namePage( width, height, nextInfo->fileName( false ) ),
                                       QString::fromLatin1( "next" ) );
        links.appendChild( link );
    }
    else
        links.appendChild( doc.createTextNode( QString::fromLatin1( "next" ) ) );

    links.appendChild( doc.createTextNode( QString::fromLatin1( " >" ) ) );

    if ( _progress->wasCancelled() )
        return false;

    // -------------------------------------------------- Resolutions
    QValueList<MyCheckBox*> actRes = activeResolutions();
    if ( actRes.count() > 1 ) {
        QDomElement resolutions = doc.createElement( QString::fromLatin1( "td" ) );
        headerRow.appendChild( resolutions );
        resolutions.setAttribute( QString::fromLatin1( "align" ), QString::fromLatin1( "right" ) );

        for( QValueList<MyCheckBox*>::Iterator sizeIt = actRes.begin();
             sizeIt != actRes.end(); ++sizeIt ) {
            int w = (*sizeIt)->width();
            int h = (*sizeIt)->height();
            QString page = namePage( w, h, info->fileName( false ) );
            QString text = (*sizeIt)->text(false);

            if ( width == w && height == h ) {
                resolutions.appendChild( doc.createTextNode( text ) );
            }
            else {
                QDomElement link = createLink( doc, page, text );
                resolutions.appendChild( link );
            }
            resolutions.appendChild( doc.createTextNode( QString::fromLatin1( " " ) ) );
        }
    }


    // -------------------------------------------------- The image
    QDomElement imgRow = doc.createElement( QString::fromLatin1( "tr" ) );
    table.appendChild( imgRow );
    QDomElement imgCol = doc.createElement( QString::fromLatin1( "td" ) );
    imgCol.setAttribute( QString::fromLatin1( "colspan" ), QString::fromLatin1( "2" ) );
    imgRow.appendChild( imgCol );

    QString name = createImage( info, width );
    QDomElement img = doc.createElement( QString::fromLatin1( "img" ) );
    img.setAttribute( QString::fromLatin1( "src" ), name );
    imgCol.appendChild( img );

    // -------------------------------------------------- Description
    table = doc.createElement( QString::fromLatin1( "table" ) );
    body.appendChild( table );

    QStringList optionGroups = Options::instance()->optionGroups();
    for( QStringList::Iterator it = optionGroups.begin(); it != optionGroups.end(); ++it ) {
        if ( info->optionValue( *it ).count() != 0 ) {
            QDomElement row = doc.createElement( QString::fromLatin1( "tr" ) );
            table.appendChild( row );

            QDomElement col = doc.createElement( QString::fromLatin1( "td" ) );
            row.appendChild( col );
            col.appendChild( doc.createTextNode( *it+ QString::fromLatin1( ":" ) ) );

            col = doc.createElement( QString::fromLatin1( "td" ) );
            row.appendChild( col );
            col.appendChild( doc.createTextNode( info->optionValue( *it ).join( QString::fromLatin1(", ") ) ) );
        }

    }

    if ( !info->description().isEmpty() ) {
        QDomElement row = doc.createElement( QString::fromLatin1( "tr" ) );
        table.appendChild( row );

        QDomElement col = doc.createElement( QString::fromLatin1( "td" ) );
        row.appendChild( col );
        col.appendChild( doc.createTextNode( QString::fromLatin1( "Description:" ) ) );

        col = doc.createElement( QString::fromLatin1( "td" ) );
        row.appendChild( col );
        col.appendChild( doc.createTextNode( info->description() ) );
    }



    // -------------------------------------------------- write to file
    QString str = doc.toString();
    QString fileName = _tempDir + namePage( width, height, info->fileName(false) );
    bool ok = writeToFile( fileName, str );
    if ( !ok )
        return false;

    return true;
}



bool HTMLExportDialog::writeToFile( const QString& fileName, const QString& str )
{
    QFile file(fileName);
    if ( !file.open(IO_WriteOnly) ) {
        KMessageBox::error( this, i18n("Couldn't create file '%1'.").arg(fileName),
                            i18n("Couldn't Create File") );
        return false;
    }

    QTextStream stream( &file );
    stream << str;
    file.close();
    return true;
}


QString HTMLExportDialog::createImage( ImageInfo* info, int size )
{
    ImageManager::instance()->load( info->fileName( false ),  this, info->angle(), size, size, false, true );
    return nameThumbNail( info, size );
}

void HTMLExportDialog::pixmapLoaded( const QString& fileName, int width, int height, int /*angle*/, const QImage& image )
{

    _waitCounter--;

    QString dir = _tempDir;
    if ( !dir.isNull() ) {
        QFileInfo finfo(fileName);
        QString baseName = finfo.baseName();
        QString ext = finfo.extension();
        int size = QMAX( width, height );

        QString file = dir + QString::fromLatin1( "/" ) + baseName;
        if ( size != -1 )
            file += QString::fromLatin1( "-" ) + QString::number( size );
        file += QString::fromLatin1( ".jpg" );

        bool success = image.save( file, "JPEG" );
        if ( !success ) {
            QMessageBox::warning( this, i18n("Unable to Write Image"), i18n("Unable to write image '%1'.").arg(file), QMessageBox::Ok, 0 );
        }
    }
    else
        Q_ASSERT( false );


    _progress->setProgress( _total - _waitCounter );
    qApp->eventLoop()->processEvents( QEventLoop::AllEvents );

    if ( _waitCounter == 0 ) {
        qApp->eventLoop()->exitLoop();
    }
}

void HTMLExportDialog::slotOk()
{
    // Progress dialog
    _progress = new QProgressDialog( i18n("Generating images for HTML page "), i18n("&Cancel"), 0, this );

    bool ok = generate();
    if ( ok ) {
        Options::instance()->setHTMLBaseDir( _baseDir->text() );
        Options::instance()->setHTMLBaseURL( _baseURL->text() );
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
    ImageManager::instance()->stop( this );
    _waitCounter = 0;
}

void HTMLExportDialog::showBrowser()
{
    if ( ! _baseURL->text().isEmpty() )
        new KRun( _baseURL->text() + QString::fromLatin1( "/" ) + _outputDir->text());
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
    bool ok = KIO::NetAccess::stat( baseDir, result, this );
#else
    bool ok = KIO::NetAccess::stat( baseDir, result );
#endif
    if ( !ok ) {
        KMessageBox::error( this, i18n("<qt>Error while reading information about %1. "
                                       "This is most likely because the directory does not exist.</qt>")
                            .arg( baseDir ) );
        return false;
    }

    KFileItem fileInfo( result, baseDir );
    if ( !fileInfo.isDir() ) {
        KMessageBox::error( this, i18n("<qt>%1 does not exist, is not a directory or "
                                       "cannot be written to.</qt>").arg( baseDir ) );
        return false;
    }


    // test if destination directory exists.
#if KDE_IS_VERSION( 3, 1, 90 )
    bool exists = KIO::NetAccess::exists( outputDir, false, this );
#else
    bool exists = KIO::NetAccess::exists( outputDir );
#endif
    if ( exists ) {
        int answer = QMessageBox::warning( this, i18n("Directory Exists"),
                                           i18n("<qt>Output directory %1 already exists. "
                                                "Usually you should specify a new directory. "
                                                "Continue?</qt>").arg( outputDir ),
                                           QMessageBox::No, QMessageBox::Yes );
        if ( answer == QMessageBox::No )
            return false;
    }
    return true;
}

int HTMLExportDialog::calculateSteps()
{
    int count = 0;
    for( QValueList<MyCheckBox*>::Iterator it2 = _cbs.begin(); it2 != _cbs.end(); ++it2 ) {
        if ( (*it2)->isChecked() )
            count++;
    }

    return _list.count() * ( 1 + count ); // 1 thumbnail + 1 real image
}

QString HTMLExportDialog::namePage( int width, int height, const QString& fileName )
{
    // PENDING(blackie) Handle situation where we are generating holiday1/me.jpg and holiday2/me.jpg
    QFileInfo fi(fileName);
    QString baseName = fi.baseName();
    return QString::fromLatin1( "%1-%2.html" ).arg( baseName ).arg(MyCheckBox::text(width,height,true));
}

QString HTMLExportDialog::nameThumbNail( ImageInfo* info, int size )
{
    // PENDING(blackie) Handle name overlap
    QFileInfo finfo(info->fileName(true));
    QString baseName = finfo.baseName();
    QString name = baseName;
    if ( size != -1 )
        name += QString::fromLatin1( "-" ) + QString::number( size );
    name += QString::fromLatin1( ".jpg" ) ;
    return name;
}

QDomElement HTMLExportDialog::createHTMLHeader( QDomDocument& doc, const QString& title )
{
    QDomElement top = doc.createElement( QString::fromLatin1( "html" ));
    doc.appendChild( top );

    QDomElement head = doc.createElement( QString::fromLatin1( "head" ) );
    top.appendChild( head );

    QDomElement titleElm = doc.createElement( QString::fromLatin1( "title" ) );
    head.appendChild( titleElm );
    QDomText text = doc.createTextNode( title );
    titleElm.appendChild( text );

    QDomElement body = doc.createElement( QString::fromLatin1( "body" ) );
    top.appendChild( body );
    body.setAttribute( QString::fromLatin1( "bgcolor" ), QString::fromLatin1( "#000000" ) );
    body.setAttribute( QString::fromLatin1( "text" ), QString::fromLatin1( "#ffffff" ) );
    return body;
}

QDomElement HTMLExportDialog::createLink( QDomDocument& doc, const QString& link, const QString& text )
{
    QDomElement href = doc.createElement( QString::fromLatin1( "a" ) );
    href.setAttribute( QString::fromLatin1( "href" ), link );
    QDomText textNode = doc.createTextNode( text );
    href.appendChild( textNode );
    return href;
}

bool HTMLExportDialog::linkIndexFile()
{
    for( QValueList<MyCheckBox*>::Iterator it = _preferredSizes.begin();
         it != _preferredSizes.end(); ++it ) {
        if ( (*it)->isChecked() ) {
            QString fromFile = QString::fromLatin1("index-%1.html" )
                               .arg((*it)->text(true));
            QString destFile = _tempDir + QString::fromLatin1("/index.html");
            bool ok = ( symlink( fromFile.latin1(), destFile.latin1() ) == 0 );
            if ( !ok ) {
                KMessageBox::error( this, i18n("<qt>Unable to make a symlink from %1 to %2</qt>")
                                    .arg( fromFile ).arg( destFile ) );

                return false;
            }
            return ok;
        }
    }
    return false;
}


QValueList<MyCheckBox*> HTMLExportDialog::activeResolutions()
{
    QValueList<MyCheckBox*> res;
    for( QValueList<MyCheckBox*>::Iterator sizeIt = _cbs.begin(); sizeIt != _cbs.end(); ++sizeIt ) {
        if ( (*sizeIt)->isChecked() ) {
            res << *sizeIt;
        }
    }
    return res;
}

void HTMLExportDialog::createSponsorLink( QDomDocument& doc, QDomElement& body )
{
    body.appendChild( doc.createElement( QString::fromLatin1( "hr" ) ) );
    body.appendChild( doc.createTextNode( QString::fromLatin1( "Sponsoring links:" ) ) );
    body.appendChild( doc.createElement( QString::fromLatin1( "br" ) ) );
    QDomElement script = doc.createElement( QString::fromLatin1( "script" ) );
    script.setAttribute( QString::fromLatin1( "type" ), QString::fromLatin1( "text/javascript" ) );
    body.appendChild( script );

    QString content = QString::fromLatin1( "\ngoogle_ad_client = \"pub-5849826863035695\";\n"
                                           "google_ad_width = 728;\n"
                                           "google_ad_height = 90;\n"
                                           "google_ad_format = \"728x90_as\";\n"
                                           "google_color_border = \"336699\";\n"
                                           "google_color_bg = \"FFFFFF\";\n"
                                           "google_color_link = \"0000FF\";\n"
                                           "google_color_url = \"008000\";\n"
                                           "google_color_text = \"000000\";\n");
    script.appendChild( doc.createComment( content ) );

    script = doc.createElement( QString::fromLatin1( "script" ) );
    script.setAttribute( QString::fromLatin1("type"), QString::fromLatin1("text/javascript") );
    script.setAttribute( QString::fromLatin1( "src" ),
                         QString::fromLatin1( "http://pagead2.googlesyndication.com/pagead/show_ads.js" ));
    body.appendChild( script );
    script.appendChild( doc.createTextNode( QString::fromLatin1( " " ) ) );
}

#include "htmlexportdialog.moc"
