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

#include "Export.h"
#include <kfiledialog.h>
#include <kzip.h>
#include <qfileinfo.h>
#include "Utilities/Util.h"
#include <QProgressDialog>
#include <klocale.h>
#include <time.h>
#include "ImageManager/AsyncLoader.h"
#include "DB/ImageInfo.h"
#include <qapplication.h>
#include <kmessagebox.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qspinbox.h>

#include <qradiobutton.h>
#include <qbuffer.h>
#include "XMLHandler.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <DB/FileNameList.h>

using namespace ImportExport;

void Export::imageExport(const DB::FileNameList& list)
{
    ExportConfig config;
    if ( config.exec() == QDialog::Rejected )
        return;

    int maxSize = -1;
    if ( config._enforeMaxSize->isChecked() )
        maxSize = config._maxSize->value();

    // Ask for zip file name
    QString zipFile = KFileDialog::getSaveFileName( KUrl(), QString::fromLatin1( "*.kim|KPhotoAlbum Export Files" ) );
    if ( zipFile.isNull() )
        return;

    bool ok;
    Export* exp = new Export( list, zipFile, config._compress->isChecked(), maxSize, config.imageFileLocation(),
                              QString::fromLatin1( "" ), config._generateThumbnails->isChecked(), &ok);
    delete exp; // It will not return before done - we still need a class to connect slots etc.

    if ( ok )
        showUsageDialog();
}

// PENDING(blackie) add warning if images are to be copied into a non empty directory.
ExportConfig::ExportConfig()
{
    setWindowTitle( i18n("Export Configuration / Copy Images") );
    setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Help );
    QWidget* top = new QWidget;
    setMainWidget( top );

    QVBoxLayout* lay1 = new QVBoxLayout( top );

    // Include images
    QGroupBox* grp = new QGroupBox( i18n("How to Handle Images") );
    lay1->addWidget( grp );

    QVBoxLayout* boxLay = new QVBoxLayout( grp );
    _include = new QRadioButton( i18n("Include in .kim file"), grp );
    _manually = new QRadioButton( i18n("Do not copy files, only generate .kim file"), grp );
    _auto = new QRadioButton( i18n("Automatically copy next to .kim file"), grp );
    _link = new QRadioButton( i18n("Hard link next to .kim file"), grp );
    _symlink = new QRadioButton( i18n("Symbolic link next to .kim file"), grp );
    _auto->setChecked( true );

    boxLay->addWidget( _include );
    boxLay->addWidget( _manually );
    boxLay->addWidget( _auto );
    boxLay->addWidget( _link );
    boxLay->addWidget( _symlink );

    // Compress
    _compress = new QCheckBox( i18n("Compress export file"), top );
    lay1->addWidget( _compress );

    // Generate thumbnails
    _generateThumbnails = new QCheckBox( i18n("Generate thumbnails"), top );
    _generateThumbnails->setChecked( false );
    lay1->addWidget( _generateThumbnails );

    // Enforece max size
    QHBoxLayout* hlay = new QHBoxLayout;
    lay1->addLayout( hlay );

    _enforeMaxSize = new QCheckBox( i18n( "Limit maximum image dimensions to: " ) );
    hlay->addWidget( _enforeMaxSize );

    _maxSize = new QSpinBox;
    _maxSize->setRange( 100,4000 );

    hlay->addWidget( _maxSize );
    _maxSize->setValue( 800 );

    connect( _enforeMaxSize, SIGNAL(toggled(bool)), _maxSize, SLOT(setEnabled(bool)) );
    _maxSize->setEnabled( false );

    QString txt = i18n( "<p>If your images are stored in a non-compressed file format then you may check this; "
                        "otherwise, this just wastes time during import and export operations.</p>"
                        "<p>In other words, do not check this if your images are stored in jpg, png or gif; but do check this "
                        "if your images are stored in tiff.</p>" );
    _compress->setWhatsThis( txt );

    txt = i18n( "<p>Generate thumbnail images</p>" );
    _generateThumbnails->setWhatsThis( txt );

    txt = i18n( "<p>With this option you may limit the maximum dimensions (width and height) of your images. "
                "Doing so will make the resulting export file smaller, but will of course also make the quality "
                "worse if someone wants to see the exported images with larger dimensions.</p>" );

    _enforeMaxSize->setWhatsThis( txt );
    _maxSize->setWhatsThis( txt );

    txt = i18n("<p>When exporting images, bear in mind that there are two things the "
               "person importing these images again will need:<br/>"
               "1) meta information (image content, date etc.)<br/>"
               "2) the images themselves.</p>"

               "<p>The images themselves can either be placed next to the .kim file, "
               "or copied into the .kim file. Copying the images into the .kim file works well "
               "for a recipient who wants all, or most of those images, for example "
               "when emailing a whole group of images. However, when you place the "
               "images on the Web, a lot of people will see them but most likely only "
               "download a few of them. It works better in this kind of case, to "
               "separate the images and the .kim file, by place them next to each "
               "other, so the user can access the images s/he wants.</p>");

    grp->setWhatsThis( txt );
    _include->setWhatsThis( txt );
    _manually->setWhatsThis( txt );
    _link->setWhatsThis( txt );
    _symlink->setWhatsThis( txt );
    _auto->setWhatsThis( txt );
    setHelp( QString::fromLatin1( "chp-exportDialog" ) );
}

ImageFileLocation ExportConfig::imageFileLocation() const
{
    if ( _include->isChecked() )
        return Inline;
    else if ( _manually->isChecked() )
        return ManualCopy;
    else if ( _link->isChecked() )
        return Link;
    else if ( _symlink->isChecked() )
        return Symlink;
    else
        return AutoCopy;
}

Export::~Export()
{
    delete _eventLoop;
}

Export::Export(
    const DB::FileNameList& list,
    const QString& zipFile,
    bool compress,
    int maxSize,
    ImageFileLocation location,
    const QString& baseUrl,
    bool doGenerateThumbnails,
    bool *ok)
    : _ok( ok )
    , _maxSize( maxSize )
    , _location( location )
    , _eventLoop( new QEventLoop )
{
    *ok = true;
    _destdir = QFileInfo( zipFile ).path();
    _zip = new KZip( zipFile );
    _zip->setCompression( compress ? KZip::DeflateCompression : KZip::NoCompression );
    if ( ! _zip->open( QIODevice::WriteOnly ) ) {
        KMessageBox::error( nullptr, i18n("Error creating zip file") );
        *ok = false;
        return;
    }

    // Create progress dialog
    int total = 1;
    if (location != ManualCopy)
      total += list.size();
    if (doGenerateThumbnails)
      total += list.size();

    _steps = 0;
    _progressDialog = new QProgressDialog;
    _progressDialog->setCancelButtonText(i18n("&Cancel"));
    _progressDialog->setMaximum(total);

    _progressDialog->setValue(0);
    _progressDialog->show();

    // Copy image files and generate thumbnails
    if ( location != ManualCopy ) {
        _copyingFiles = true;
        copyImages( list );
    }

    if ( _ok && doGenerateThumbnails ) {
        _copyingFiles = false;
        generateThumbnails( list );
    }

    if ( _ok ) {
        // Create the index.xml file
        _progressDialog->setLabelText(i18n("Creating index file"));
        QByteArray indexml = XMLHandler().createIndexXML( list, baseUrl, _location, &_filenameMapper );
        time_t t;
        time(&t);
        _zip->writeFile( QString::fromLatin1( "index.xml" ), QString(), QString(), indexml.data(), indexml.size()-1 );

       _steps++;
       _progressDialog->setValue( _steps );
        _zip->close();
    }
}


void Export::generateThumbnails(const DB::FileNameList& list)
{
    _progressDialog->setLabelText( i18n("Creating thumbnails") );
    _loopEntered = false;
    _subdir = QString::fromLatin1( "Thumbnails/" );
    _filesRemaining = list.size(); // Used to break the event loop.
    for (const DB::FileName& fileName : list) {
        ImageManager::ImageRequest* request = new ImageManager::ImageRequest( fileName, QSize( 128, 128 ), fileName.info()->angle(), this );
        request->setPriority( ImageManager::BatchTask );
        ImageManager::AsyncLoader::instance()->load( request );
    }
    if ( _filesRemaining > 0 ) {
        _loopEntered = true;
        _eventLoop->exec();
    }
}

void Export::copyImages(const DB::FileNameList& list)
{
    Q_ASSERT( _location != ManualCopy );

    _loopEntered = false;
    _subdir = QString::fromLatin1( "Images/" );

    _progressDialog->setLabelText( i18n("Copying image files") );

    _filesRemaining = 0;
    for (const DB::FileName& fileName : list) {
        QString file = fileName.absolute();
        QString zippedName = _filenameMapper.uniqNameFor(fileName);

        if ( _maxSize == -1 || Utilities::isVideo( fileName ) || Utilities::isRAW( fileName )) {
            if ( QFileInfo( file ).isSymLink() )
                file = QFileInfo(file).readLink();

            if ( _location == Inline )
                _zip->addLocalFile( file, QString::fromLatin1( "Images/" ) + zippedName );
            else if ( _location == AutoCopy )
                Utilities::copy( file, _destdir + QString::fromLatin1( "/" ) + zippedName );
            else if ( _location == Link )
                Utilities::makeHardLink( file, _destdir + QString::fromLatin1( "/" ) + zippedName );
            else if ( _location == Symlink )
                Utilities::makeSymbolicLink( file, _destdir + QString::fromLatin1( "/" ) + zippedName );

            _steps++;
            _progressDialog->setValue( _steps );
        }
        else {
            _filesRemaining++;
            ImageManager::ImageRequest* request =
                new ImageManager::ImageRequest( DB::FileName::fromAbsolutePath(file), QSize( _maxSize, _maxSize ), 0, this );
            request->setPriority( ImageManager::BatchTask );
            ImageManager::AsyncLoader::instance()->load( request );
        }

        // Test if the cancel button was pressed.
        qApp->processEvents( QEventLoop::AllEvents );

        if ( _progressDialog->wasCanceled() ) {
            *_ok = false;
            return;
        }
    }
    if ( _filesRemaining > 0 ) {
        _loopEntered = true;
        _eventLoop->exec();
    }
}

void Export::pixmapLoaded(ImageManager::ImageRequest* request, const QImage& image)
{
    const DB::FileName fileName = request->databaseFileName();
    if ( !request->loadedOK() )
        return;

    const QString ext = (Utilities::isVideo( fileName ) || Utilities::isRAW( fileName )) ? QString::fromLatin1( "jpg" ) : QFileInfo( _filenameMapper.uniqNameFor(fileName) ).completeSuffix();

    // Add the file to the zip archive
    QString zipFileName = QString::fromLatin1( "%1/%2.%3" ).arg( Utilities::stripEndingForwardSlash(_subdir))
        .arg(QFileInfo( _filenameMapper.uniqNameFor(fileName) ).baseName()).arg( ext );
    QByteArray data;
    QBuffer buffer( &data );
    buffer.open( QIODevice::WriteOnly );
    image.save( &buffer,  QFileInfo(zipFileName).suffix().toLower().toLatin1() );

    if ( _location == Inline || !_copyingFiles )
        _zip->writeFile( zipFileName, QString(), QString(), data, data.size() );
    else {
        QString file = _destdir + QString::fromLatin1( "/" ) + _filenameMapper.uniqNameFor(fileName);
        QFile out( file );
        if ( !out.open( QIODevice::WriteOnly ) ) {
            KMessageBox::error( nullptr, i18n("Error writing file %1", file ) );
            *_ok = false;
        }
        out.write( data, data.size() );
        out.close();
    }

    qApp->processEvents( QEventLoop::AllEvents );

    bool canceled = (!_ok ||  _progressDialog->wasCanceled());

    if ( canceled ) {
        *_ok = false;
        _eventLoop->exit();
        ImageManager::AsyncLoader::instance()->stop( this );
        return;
    }

    _steps++;
    _filesRemaining--;
    _progressDialog->setValue( _steps );


        if ( _filesRemaining == 0 && _loopEntered )
            _eventLoop->exit();
}

void Export::showUsageDialog()
{
    QString txt =
        i18n( "<p>Other KPhotoAlbum users may now load the import file into their database, by choosing <b>import</b> in "
              "the file menu.</p>"
              "<p>If they find it on a web site, and the web server is correctly configured, all they need to do is simply "
              "to click it from within konqueror. To enable this, your web server needs to be configured for KPhotoAlbum. You do so by adding "
              "the following line to <b>/etc/httpd/mime.types</b> or similar:"
              "<pre>application/vnd.kde.kphotoalbum-import kim</pre>"
              "This will make your web server tell konqueror that it is a KPhotoAlbum file when clicking on the link, "
              "otherwise the web server will just tell konqueror that it is a plain text file.</p>" );

    KMessageBox::information( nullptr, txt, i18n("How to Use the Export File"), QString::fromLatin1("export_how_to_use_the_export_file") );
}




#include "Export.moc"
// vi:expandtab:tabstop=4 shiftwidth=4:
