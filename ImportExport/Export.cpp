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

#include "Export.h"
#include <kfiledialog.h>
#include <kzip.h>
#include <qfileinfo.h>
#include <time.h>
#include "Utilities/Util.h"
#include <qprogressdialog.h>
#include <qeventloop.h>
#include <klocale.h>
#include "ImageManager/Manager.h"
#include <qapplication.h>
#include <kmessagebox.h>
#include <kdialogbase.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qwhatsthis.h>
#include <qvbuttongroup.h>
#include <qradiobutton.h>
#include <kimageio.h>
#include "DB/ImageDB.h"
#include <qbuffer.h>
#include "XMLHandler.h"

using namespace ImportExport;

void Export::imageExport( const QStringList& list )
{
    ExportConfig config;
    if ( config.exec() == QDialog::Rejected )
        return;

    int maxSize = -1;
    if ( config._enforeMaxSize->isChecked() )
        maxSize = config._maxSize->value();

    // Ask for zip file name
    QString zipFile = KFileDialog::getSaveFileName( QString::null, QString::fromLatin1( "*.kim|KPhotoAlbum Export Files" ), 0 );
    if ( zipFile.isNull() )
        return;

    bool ok;
    Export* exp = new Export( list, zipFile, config._compress->isChecked(), maxSize, config.imageFileLocation(),
                              QString::fromLatin1( "" ), ok, config._generateThumbnails->isChecked());
    delete exp; // It will not return before done - we still need a class to connect slots etc.

    if ( ok )
        showUsageDialog();
}

// PENDING(blackie) add warning if images are to be copied into a non empty directory.
ExportConfig::ExportConfig()
    : KDialogBase( KDialogBase::Plain, i18n("Export Configuration/Copy Images"), KDialogBase::Ok | KDialogBase::Cancel | KDialogBase::Help,
                   KDialogBase::Ok, 0, "export config" )
{
    QWidget* top = plainPage();
    QVBoxLayout* lay1 = new QVBoxLayout( top, 6 );

    // Include images
    QVButtonGroup* grp = new QVButtonGroup( i18n("How to Handle Images"), top );
    lay1->addWidget( grp );
    _include = new QRadioButton( i18n("Include in .kim file"), grp );
    _manually = new QRadioButton( i18n("Manual copy next to .kim file"), grp );
    _auto = new QRadioButton( i18n("Automatically copy next to .kim file"), grp );
    _link = new QRadioButton( i18n("Hard link next to .kim file"), grp );
    _manually->setChecked( true );

    // Compress
    _compress = new QCheckBox( i18n("Compress export file"), top );
    lay1->addWidget( _compress );

    // Generate thumbnails
    _generateThumbnails = new QCheckBox( i18n("Generate thumbnails"), top );
    _generateThumbnails->setChecked(true);
    lay1->addWidget( _generateThumbnails );

    // Enforece max size
    QHBoxLayout* hlay = new QHBoxLayout( lay1, 6 );
    _enforeMaxSize = new QCheckBox( i18n( "Limit maximum image dimensions to: " ), top, "_enforeMaxSize" );
    hlay->addWidget( _enforeMaxSize );

    _maxSize = new QSpinBox( 100, 4000, 50, top, "_maxSize" );
    hlay->addWidget( _maxSize );
    _maxSize->setValue( 800 );

    connect( _enforeMaxSize, SIGNAL( toggled( bool ) ), _maxSize, SLOT( setEnabled( bool ) ) );
    _maxSize->setEnabled( false );

    QString txt = i18n( "<p>If your images are stored in a non-compressed file format then you may check this; "
                        "otherwise, this just wastes time during import and export operations.</p>"
                        "<p>In other words, do not check this if your images are stored in jpg, png or gif; but do check this "
                        "if your images are stored in tiff.</p>" );
    QWhatsThis::add( _compress, txt );

    txt = i18n( "<p>Generate thumbnail images</p>" );
    QWhatsThis::add( _generateThumbnails, txt );

    txt = i18n( "<p>With this option you may limit the maximum dimensions (width and height) of your images. "
                "Doing so will make the resulting export file smaller, but will of course also make the quality "
                "worse if someone wants to see the exported images with larger dimensions.</p>" );

    QWhatsThis::add( _enforeMaxSize, txt );
    QWhatsThis::add( _maxSize, txt );

    txt = i18n("<p>When exporting images, bear in mind that there are two things the "
               "person importing these images again will need:<br>"
               "1) meta information (image content, date etc.)<br>"
               "2) the images themselves.</p>"

               "<p>The images themselves can either be placed next to the .kim file, "
               "or copied into the .kim file. Copying the images into the .kim file works well "
               "for a recipient who wants all, or most of those images, for example "
               "when emailing a whole group of images. However, when you place the "
               "images on the Web, a lot of people will see them but most likely only "
               "download a few of them. It works better in this kind of case, to "
               "separate the images and the .kim file, by place them next to each "
               "other, so the user can access the images s/he wants.</p>");

    QWhatsThis::add( grp, txt );
    QWhatsThis::add( _include, txt );
    QWhatsThis::add( _manually, txt );
    QWhatsThis::add( _link, txt );
    QWhatsThis::add( _auto, txt );
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
    else
        return AutoCopy;
}


Export::Export( const QStringList& list, const QString& zipFile, bool compress, int maxSize, ImageFileLocation location,
                const QString& baseUrl, bool& ok, bool doGenerateThumbnails )
    : _ok( ok ), _maxSize( maxSize ), _location( location )
{
    ok = true;
    _destdir = QFileInfo( zipFile ).dirPath();
    _zip = new KZip( zipFile );
    _zip->setCompression( compress ? KZip::DeflateCompression : KZip::NoCompression );
    if ( ! _zip->open( IO_WriteOnly ) ) {
        KMessageBox::error( 0, i18n("Error creating zip file") );
        ok = false;
        return;
    }

    // Create progress dialog
    int total = 1;
    if (location != ManualCopy)
      total += list.count();
    if (doGenerateThumbnails)
      total += list.count();

    _steps = 0;
    _progressDialog = new QProgressDialog( QString::null, i18n("&Cancel"), total, 0, "progress dialog", true );
    _progressDialog->setProgress( 0 );
    _progressDialog->show();

    _nameMap = Utilities::createUniqNameMap( list, false, QString::null );

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
        QCString indexml = XMLHandler().createIndexXML( list, baseUrl, _location, _nameMap );
        time_t t;
        time(&t);
        _zip->writeFile( QString::fromLatin1( "index.xml" ), QString::null, QString::null, indexml.size()-1,
                         0444, t, t, t, indexml.data() );

       _steps++;
       _progressDialog->setProgress( _steps );
        _zip->close();
    }
}


void Export::generateThumbnails( const QStringList& list )
{
    _progressDialog->setLabelText( i18n("Creating thumbnails") );
    _loopEntered = false;
    _subdir = QString::fromLatin1( "Thumbnails/" );
    _filesRemaining = list.count(); // Used to break the event loop.
    for( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
        ImageManager::ImageRequest* request = new ImageManager::ImageRequest( *it, QSize( 128, 128 ), DB::ImageDB::instance()->info(*it)->angle(), this );
        request->setPriority();
        ImageManager::Manager::instance()->load( request );
    }
    if ( _filesRemaining > 0 ) {
        _loopEntered = true;
        qApp->eventLoop()->enterLoop();
    }
}

void Export::copyImages( const QStringList& list )
{
    Q_ASSERT( _location != ManualCopy );

    _loopEntered = false;
    _subdir = QString::fromLatin1( "Images/" );

    _progressDialog->setLabelText( i18n("Copying image files") );

    _filesRemaining = 0;
    for( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
        QString file = *it;
        QString zippedName = _nameMap[file];

        if ( _maxSize == -1 || Utilities::isVideo( file ) ) {
            if ( QFileInfo( file ).isSymLink() )
                file = QFileInfo(file).readLink();

            if ( _location == Inline )
                _zip->addLocalFile( file, QString::fromLatin1( "Images/" ) + zippedName );
            else if ( _location == AutoCopy )
                Utilities::copy( file, _destdir + QString::fromLatin1( "/" ) + zippedName );
            else if ( _location == Link )
                Utilities::makeHardLink( file, _destdir + QString::fromLatin1( "/" ) + zippedName );

            _steps++;
            _progressDialog->setProgress( _steps );
        }
        else {
            _filesRemaining++;
            ImageManager::ImageRequest* request =
                new ImageManager::ImageRequest( *it, QSize( _maxSize, _maxSize ), DB::ImageDB::instance()->info(*it)->angle(), this );
            request->setPriority();
            ImageManager::Manager::instance()->load( request );
        }

        // Test if the cancel button was pressed.
        qApp->eventLoop()->processEvents( QEventLoop::AllEvents );

        if ( _progressDialog->wasCanceled() ) {
            _ok = false;
            return;
        }
    }
    if ( _filesRemaining > 0 ) {
        _loopEntered = true;
        qApp->eventLoop()->enterLoop();
    }
}

void Export::pixmapLoaded( const QString& fileName, const QSize& /*size*/, const QSize& /*fullSize*/, int /*angle*/, const QImage& image, bool loadedOK )
{
    if ( !loadedOK )
        return;

    const QString ext = Utilities::isVideo( fileName ) ? QString::fromLatin1( "jpg" ) : QFileInfo( _nameMap[fileName] ).extension();

    // Add the file to the zip archive
    QString zipFileName = QString::fromLatin1( "%1/%2.%3" ).arg( Utilities::stripSlash(_subdir))
                          .arg(QFileInfo( _nameMap[fileName] ).baseName()).arg( ext );
    QByteArray data;
    QBuffer buffer( data );
    buffer.open( IO_WriteOnly );
    image.save( &buffer, QFile::encodeName( KImageIO::type( zipFileName ) ) );

    if ( _location == Inline || !_copyingFiles )
        _zip->writeFile( zipFileName, QString::null, QString::null, data.size(), data );
    else {
        QString file = _destdir + QString::fromLatin1( "/" ) + _nameMap[fileName];
        QFile out( file );
        if ( !out.open( IO_WriteOnly ) ) {
            KMessageBox::error( 0, i18n("Error writing file %1").arg( file ) );
            _ok = false;
        }
        out.writeBlock( data, data.size() );
        out.close();
    }

    qApp->eventLoop()->processEvents( QEventLoop::AllEvents );

    bool canceled = (!_ok ||  _progressDialog->wasCanceled());

    if ( canceled ) {
        _ok = false;
        qApp->eventLoop()->exitLoop();
        ImageManager::Manager::instance()->stop( this );
        return;
    }

    _steps++;
    _filesRemaining--;
    _progressDialog->setProgress( _steps );

    if ( _filesRemaining == 0 && _loopEntered )
        qApp->eventLoop()->exitLoop();
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

    KMessageBox::information( 0, txt, i18n("How to Use the Export File"), QString::fromLatin1("export_how_to_use_the_export_file") );
}




#include "Export.moc"
