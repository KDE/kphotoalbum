/* Copyright (C) 2003-2018 Jesper K. Pedersen <blackie@kde.org>

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

#include <QApplication>
#include <QBuffer>
#include <QCheckBox>
#include <QFileInfo>
#include <QGroupBox>
#include <QLayout>
#include <QProgressDialog>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QFileDialog>

#include <KHelpClient>
#include <KLocalizedString>
#include <KMessageBox>
#include <KZip>

#include <DB/FileNameList.h>
#include <DB/ImageInfo.h>
#include <ImageManager/AsyncLoader.h>
#include <Utilities/Util.h>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>

#include "XMLHandler.h"

using namespace ImportExport;

void Export::imageExport(const DB::FileNameList& list)
{
    ExportConfig config;
    if ( config.exec() == QDialog::Rejected )
        return;

    int maxSize = -1;
    if ( config.mp_enforeMaxSize->isChecked() )
        maxSize = config.mp_maxSize->value();

    // Ask for zip file name
    QString zipFile = QFileDialog::getSaveFileName(
                nullptr, /* parent */
                i18n("Save an export file"), /* caption */
                QString(), /* directory */
                i18n("KPhotoAlbum import files") +
                QString::fromLatin1( "(*.kim)" ) /*filter*/
                );
    if ( zipFile.isNull() )
        return;

    bool ok;
    Export* exp = new Export( list, zipFile, config.mp_compress->isChecked(), maxSize, config.imageFileLocation(),
                              QString::fromLatin1( "" ), config.mp_generateThumbnails->isChecked(), &ok);
    delete exp; // It will not return before done - we still need a class to connect slots etc.

    if ( ok )
        showUsageDialog();
}

// PENDING(blackie) add warning if images are to be copied into a non empty directory.
ExportConfig::ExportConfig()
{
    setWindowTitle( i18nc("@title:window", "Export Configuration / Copy Images") );
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::Help);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);

    QWidget* top = new QWidget;
    mainLayout->addWidget(top);

    QVBoxLayout* lay1 = new QVBoxLayout( top );

    // Include images
    QGroupBox* grp = new QGroupBox( i18n("How to Handle Images") );
    lay1->addWidget( grp );

    QVBoxLayout* boxLay = new QVBoxLayout( grp );
    m_include = new QRadioButton( i18n("Include in .kim file"), grp );
    m_manually = new QRadioButton( i18n("Do not copy files, only generate .kim file"), grp );
    m_auto = new QRadioButton( i18n("Automatically copy next to .kim file"), grp );
    m_link = new QRadioButton( i18n("Hard link next to .kim file"), grp );
    m_symlink = new QRadioButton( i18n("Symbolic link next to .kim file"), grp );
    m_auto->setChecked( true );

    boxLay->addWidget( m_include );
    boxLay->addWidget( m_manually );
    boxLay->addWidget( m_auto );
    boxLay->addWidget( m_link );
    boxLay->addWidget( m_symlink );

    // Compress
    mp_compress = new QCheckBox( i18n("Compress export file"), top );
    lay1->addWidget( mp_compress );

    // Generate thumbnails
    mp_generateThumbnails = new QCheckBox( i18n("Generate thumbnails"), top );
    mp_generateThumbnails->setChecked( false );
    lay1->addWidget( mp_generateThumbnails );

    // Enforece max size
    QHBoxLayout* hlay = new QHBoxLayout;
    lay1->addLayout( hlay );

    mp_enforeMaxSize = new QCheckBox( i18n( "Limit maximum image dimensions to: " ) );
    hlay->addWidget( mp_enforeMaxSize );

    mp_maxSize = new QSpinBox;
    mp_maxSize->setRange( 100,4000 );

    hlay->addWidget( mp_maxSize );
    mp_maxSize->setValue( 800 );

    connect(mp_enforeMaxSize, &QCheckBox::toggled, mp_maxSize, &QSpinBox::setEnabled);
    mp_maxSize->setEnabled( false );

    QString txt = i18n( "<p>If your images are stored in a non-compressed file format then you may check this; "
                        "otherwise, this just wastes time during import and export operations.</p>"
                        "<p>In other words, do not check this if your images are stored in jpg, png or gif; but do check this "
                        "if your images are stored in tiff.</p>" );
    mp_compress->setWhatsThis( txt );

    txt = i18n( "<p>Generate thumbnail images</p>" );
    mp_generateThumbnails->setWhatsThis( txt );

    txt = i18n( "<p>With this option you may limit the maximum dimensions (width and height) of your images. "
                "Doing so will make the resulting export file smaller, but will of course also make the quality "
                "worse if someone wants to see the exported images with larger dimensions.</p>" );

    mp_enforeMaxSize->setWhatsThis( txt );
    mp_maxSize->setWhatsThis( txt );

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
    m_include->setWhatsThis( txt );
    m_manually->setWhatsThis( txt );
    m_link->setWhatsThis( txt );
    m_symlink->setWhatsThis( txt );
    m_auto->setWhatsThis( txt );

    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ExportConfig::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ExportConfig::reject);
    mainLayout->addWidget(buttonBox);

    QPushButton *helpButton = buttonBox->button(QDialogButtonBox::Help);
    connect(helpButton,&QPushButton::clicked, this,&ExportConfig::showHelp);
}

ImageFileLocation ExportConfig::imageFileLocation() const
{
    if ( m_include->isChecked() )
        return Inline;
    else if ( m_manually->isChecked() )
        return ManualCopy;
    else if ( m_link->isChecked() )
        return Link;
    else if ( m_symlink->isChecked() )
        return Symlink;
    else
        return AutoCopy;
}

void ExportConfig::showHelp()
{
    KHelpClient::invokeHelp( QString::fromLatin1( "chp-importExport" ) );
}

Export::~Export()
{
    delete m_eventLoop;
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
    : m_internalOk(true)
    , m_ok( ok )
    , m_maxSize( maxSize )
    , m_location( location )
    , m_eventLoop( new QEventLoop )
{
    if (ok == nullptr)
        ok = &m_internalOk;
    *ok = true;
    m_destdir = QFileInfo( zipFile ).path();
    m_zip = new KZip( zipFile );
    m_zip->setCompression( compress ? KZip::DeflateCompression : KZip::NoCompression );
    if ( ! m_zip->open( QIODevice::WriteOnly ) ) {
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

    m_steps = 0;
    m_progressDialog = new QProgressDialog;
    m_progressDialog->setCancelButtonText(i18n("&Cancel"));
    m_progressDialog->setMaximum(total);

    m_progressDialog->setValue(0);
    m_progressDialog->show();

    // Copy image files and generate thumbnails
    if ( location != ManualCopy ) {
        m_copyingFiles = true;
        copyImages( list );
    }

    if ( *m_ok && doGenerateThumbnails ) {
        m_copyingFiles = false;
        generateThumbnails( list );
    }

    if ( *m_ok ) {
        // Create the index.xml file
        m_progressDialog->setLabelText(i18n("Creating index file"));
        QByteArray indexml = XMLHandler().createIndexXML( list, baseUrl, m_location, &m_filenameMapper );
        m_zip->writeFile( QString::fromLatin1( "index.xml" ), indexml.data() );

       m_steps++;
       m_progressDialog->setValue( m_steps );
        m_zip->close();
    }
}


void Export::generateThumbnails(const DB::FileNameList& list)
{
    m_progressDialog->setLabelText( i18n("Creating thumbnails") );
    m_loopEntered = false;
    m_subdir = QString::fromLatin1( "Thumbnails/" );
    m_filesRemaining = list.size(); // Used to break the event loop.
    for (const DB::FileName& fileName : list) {
        ImageManager::ImageRequest* request = new ImageManager::ImageRequest( fileName, QSize( 128, 128 ), fileName.info()->angle(), this );
        request->setPriority( ImageManager::BatchTask );
        ImageManager::AsyncLoader::instance()->load( request );
    }
    if ( m_filesRemaining > 0 ) {
        m_loopEntered = true;
        m_eventLoop->exec();
    }
}

void Export::copyImages(const DB::FileNameList& list)
{
    Q_ASSERT( m_location != ManualCopy );

    m_loopEntered = false;
    m_subdir = QString::fromLatin1( "Images/" );

    m_progressDialog->setLabelText( i18n("Copying image files") );

    m_filesRemaining = 0;
    for (const DB::FileName& fileName : list) {
        QString file = fileName.absolute();
        QString zippedName = m_filenameMapper.uniqNameFor(fileName);

        if ( m_maxSize == -1 || Utilities::isVideo( fileName ) || Utilities::isRAW( fileName )) {
            if ( QFileInfo( file ).isSymLink() )
                file = QFileInfo(file).readLink();

            if ( m_location == Inline )
                m_zip->addLocalFile( file, QString::fromLatin1( "Images/" ) + zippedName );
            else if ( m_location == AutoCopy )
                Utilities::copy( file, m_destdir + QString::fromLatin1( "/" ) + zippedName );
            else if ( m_location == Link )
                Utilities::makeHardLink( file, m_destdir + QString::fromLatin1( "/" ) + zippedName );
            else if ( m_location == Symlink )
                Utilities::makeSymbolicLink( file, m_destdir + QString::fromLatin1( "/" ) + zippedName );

            m_steps++;
            m_progressDialog->setValue( m_steps );
        }
        else {
            m_filesRemaining++;
            ImageManager::ImageRequest* request =
                new ImageManager::ImageRequest( DB::FileName::fromAbsolutePath(file), QSize( m_maxSize, m_maxSize ), 0, this );
            request->setPriority( ImageManager::BatchTask );
            ImageManager::AsyncLoader::instance()->load( request );
        }

        // Test if the cancel button was pressed.
        qApp->processEvents( QEventLoop::AllEvents );

        if ( m_progressDialog->wasCanceled() ) {
            *m_ok = false;
            return;
        }
    }
    if ( m_filesRemaining > 0 ) {
        m_loopEntered = true;
        m_eventLoop->exec();
    }
}

void Export::pixmapLoaded(ImageManager::ImageRequest* request, const QImage& image)
{
    const DB::FileName fileName = request->databaseFileName();
    if ( !request->loadedOK() )
        return;

    const QString ext = (Utilities::isVideo( fileName ) || Utilities::isRAW( fileName )) ? QString::fromLatin1( "jpg" ) : QFileInfo( m_filenameMapper.uniqNameFor(fileName) ).completeSuffix();

    // Add the file to the zip archive
    QString zipFileName = QString::fromLatin1( "%1/%2.%3" ).arg( Utilities::stripEndingForwardSlash(m_subdir))
        .arg(QFileInfo( m_filenameMapper.uniqNameFor(fileName) ).baseName()).arg( ext );
    QByteArray data;
    QBuffer buffer( &data );
    buffer.open( QIODevice::WriteOnly );
    image.save( &buffer,  QFileInfo(zipFileName).suffix().toLower().toLatin1().constData() );

    if ( m_location == Inline || !m_copyingFiles )
        m_zip->writeFile( zipFileName, data.constData() );
    else {
        QString file = m_destdir + QString::fromLatin1( "/" ) + m_filenameMapper.uniqNameFor(fileName);
        QFile out( file );
        if ( !out.open( QIODevice::WriteOnly ) ) {
            KMessageBox::error( nullptr, i18n("Error writing file %1", file ) );
            *m_ok = false;
        }
        out.write( data.constData(), data.size() );
        out.close();
    }

    qApp->processEvents( QEventLoop::AllEvents );

    bool canceled = (!*m_ok ||  m_progressDialog->wasCanceled());

    if ( canceled ) {
        *m_ok = false;
        m_eventLoop->exit();
        ImageManager::AsyncLoader::instance()->stop( this );
        return;
    }

    m_steps++;
    m_filesRemaining--;
    m_progressDialog->setValue( m_steps );


        if ( m_filesRemaining == 0 && m_loopEntered )
            m_eventLoop->exit();
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

// vi:expandtab:tabstop=4 shiftwidth=4:
