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
#include "HTMLDialog.h"
#include <klocale.h>
#include <qlayout.h>
#include <klineedit.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <kfiledialog.h>
#include <qpushbutton.h>
#include "Settings/SettingsData.h"
#include <qhgroupbox.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kfileitem.h>
#include <kio/netaccess.h>
#include <qtextedit.h>
#include <kdebug.h>
#include <qvgroupbox.h>
#include <kglobal.h>
#include <kiconloader.h>
#include "MainWindow/Window.h"
#include "DB/CategoryCollection.h"
#include "DB/ImageDB.h"
#include "Generator.h"
#include "ImageSizeCheckBox.h"
using namespace HTMLGenerator;


HTMLDialog::HTMLDialog( QWidget* parent, const char* name )
    :KDialogBase( IconList, i18n("HTML Export"), Ok|Cancel|Help, Ok, parent, name )
{
    enableButtonOK( false );
    createContentPage();
    createLayoutPage();
    createDestinationPage();
    setHelp( QString::fromLatin1( "chp-generating-html" ) );
}

void HTMLDialog::createContentPage()
{
    QWidget* contentPage = addPage( i18n("Content" ), i18n("Content" ),
                                    KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "edit" ),
                                                                     KIcon::Desktop, 32 ));
    QVBoxLayout* lay1 = new QVBoxLayout( contentPage, 6 );
    QGridLayout* lay2 = new QGridLayout( lay1, 2 );

    QLabel* label = new QLabel( i18n("Page title:"), contentPage );
    lay2->addWidget( label, 0, 0 );
    _title = new KLineEdit( contentPage );
    label->setBuddy( _title );
    lay2->addWidget( _title, 0, 1 );

    // Description
    label = new QLabel( i18n("Description:"), contentPage );
    label->setAlignment( Qt::AlignTop );
    lay2->addWidget( label, 1, 0 );
    _description = new QTextEdit( contentPage );
    label->setBuddy( _description );
    lay2->addWidget( _description, 1, 1 );

    _generateKimFile = new QCheckBox( i18n("Create .kim export file"), contentPage );
    _generateKimFile->setChecked( true );
    lay1->addWidget( _generateKimFile );

    _inlineMovies = new QCheckBox( i18n( "Inline Movies in pages" ), contentPage );
    _inlineMovies->setChecked( true );
    lay1->addWidget( _inlineMovies );

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

void HTMLDialog::createLayoutPage()
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
    label->setBuddy( _thumbSize );

    // Number of columns
    label = new QLabel( i18n("Number of columns:"), layoutPage );
    lay2->addWidget( label, 1, 0 );

    QHBoxLayout* lay4 = new QHBoxLayout( 0 );
    lay2->addLayout( lay4, 1, 1 );
    _numOfCols = new QSpinBox( 1, 10, 1, layoutPage );
    label->setBuddy( _numOfCols);

    _numOfCols->setValue( 5 );
    lay4->addWidget( _numOfCols );
    lay4->addStretch( 1 );

    // Theme box
    label = new QLabel( i18n("Theme:"), layoutPage );
    lay2->addWidget( label, 2, 0 );
    lay4 = new QHBoxLayout( 0 );
    lay2->addLayout( lay4, 2, 1 );
    _themeBox = new QComboBox( layoutPage, "theme_combobox" );
    label->setBuddy( _themeBox );
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

    _cbs << size800 << size1024 << size1280 << size640 << size1600 << size320 << sizeOrig;

    lay1->addStretch(1);
}

void HTMLDialog::createDestinationPage()
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
    label->setBuddy( _baseDir );

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
    label->setBuddy( _baseURL );

    // Destination URL
    label = new QLabel( i18n("URL for final destination:" ), destinationPage );
    lay2->addWidget( label, 2, 0 );
    _destURL = new KLineEdit( destinationPage );
    _destURL->setText( Settings::SettingsData::instance()->HTMLDestURL() );
    lay2->addWidget( _destURL, 2, 1 );
    label->setBuddy( _destURL );

    // Output Directory
    label = new QLabel( i18n("Output directory:"), destinationPage );
    lay2->addWidget( label, 3, 0 );
    _outputDir = new KLineEdit( destinationPage );
    lay2->addWidget( _outputDir, 3, 1 );
    label->setBuddy( _outputDir );

    label = new QLabel( i18n("<b>Hint: Press the help button for descriptions of the fields</b>"), destinationPage );
    lay1->addWidget( label );
    lay1->addStretch( 1 );
}

void HTMLDialog::slotOk()
{
    if ( !checkVars() )
        return;

    if( activeResolutions().count() < 1 ) {
        KMessageBox::error( 0, i18n( "You must select at least one resolution." ) );
        return;
    }

    accept();

    Settings::SettingsData::instance()->setHTMLBaseDir( _baseDir->text() );
    Settings::SettingsData::instance()->setHTMLBaseURL( _baseURL->text() );
    Settings::SettingsData::instance()->setHTMLDestURL( _destURL->text() );

    Generator generator( setup(), this );
    generator.generate();
}

void HTMLDialog::selectDir()
{
    KURL dir = KFileDialog::getExistingURL( _baseDir->text(), this );
    if ( !dir.url().isNull() )
        _baseDir->setText( dir.url() );
}

bool HTMLDialog::checkVars()
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
    bool exists = KIO::NetAccess::exists( KURL(outputDir), false, MainWindow::Window::theMainWindow() );
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
            KIO::NetAccess::del( KURL(outputDir), MainWindow::Window::theMainWindow() );
        }
        else
            return false;
    }
    return true;
}

QValueList<ImageSizeCheckBox*> HTMLDialog::activeResolutions() const
{
    QValueList<ImageSizeCheckBox*> res;
    for( QValueList<ImageSizeCheckBox*>::ConstIterator sizeIt = _cbs.begin(); sizeIt != _cbs.end(); ++sizeIt ) {
        if ( (*sizeIt)->isChecked() )
            res << *sizeIt;
    }
    return res;
}

void HTMLDialog::populateThemesCombo()
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

int HTMLDialog::exec( const QStringList& list )
{
    _list = list;
    return KDialogBase::exec();
}



Setup HTMLGenerator::HTMLDialog::setup() const
{
    Setup setup;
    setup.setTitle( _title->text() );
    setup.setBaseDir( _baseDir->text() );
    setup.setBaseURL( _baseURL->text() );
    setup.setOutputDir( _outputDir->text() );
    setup.setThumbSize( _thumbSize->value() );
    setup.setDescription( _description->text() );
    setup.setNumOfCols( _numOfCols->value() );
    setup.setGenerateKimFile( _generateKimFile->isChecked() );
    setup.setThemePath( _themes[_themeBox->currentItem()] );
    for( QMap<QString,QCheckBox*>::ConstIterator includeIt = _whatToIncludeMap.begin();
         includeIt != _whatToIncludeMap.end(); ++includeIt ) {
        setup.setIncludeCategory( includeIt.key(), includeIt.data()->isChecked() );
    }
    setup.setImageList( _list );

    setup.setResolutions( activeResolutions() );
    setup.setInlineMovies( _inlineMovies->isChecked() );
    return setup;
}

#include "HTMLDialog.moc"
