/* Copyright (C) 2003-2016 Jesper K. Pedersen <blackie@kde.org>

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

#include "HTMLDialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QScopedPointer>
#include <QSpinBox>
#include <QStandardPaths>
#include <QStringMatcher>
#include <QVBoxLayout>

#include <KConfig>
#include <KConfigGroup>
#include <KFileItem>
#include <KIO/DeleteJob>
#include <KIO/StatJob>
#include <KJob>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>
#include <KTextEdit>

#include <DB/CategoryCollection.h>
#include <DB/ImageDB.h>
#include <MainWindow/Window.h>
#include <Settings/SettingsData.h>

#include "Generator.h"
#include "ImageSizeCheckBox.h"

using namespace HTMLGenerator;

#ifdef DEBUG_HTMLGENERATOR
#define Debug qDebug
#else
#define Debug if(0) qDebug
#endif

HTMLDialog::HTMLDialog( QWidget* parent )
   : KPageDialog(parent)
   , m_list()
{
    setWindowTitle( i18n("HTML Export") );
    QWidget *mainWidget = new QWidget(this);
    this->layout()->addWidget(mainWidget);

    createContentPage();
    createLayoutPage();
    createDestinationPage();
    // destUrl is only relevant for .kim file creation:
    connect(m_generateKimFile,&QCheckBox::toggled,m_destURL,&QLineEdit::setEnabled);
    // automatically fill in output directory:
    connect(m_title,&QLineEdit::editingFinished,this,&HTMLDialog::slotSuggestOutputDir);

    QDialogButtonBox *buttonBox = this->buttonBox();
    connect(buttonBox, &QDialogButtonBox::accepted, this, &HTMLDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &HTMLDialog::reject);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    okButton->setEnabled( m_themes.size()>0 );
    connect(okButton, &QPushButton::clicked, this, &HTMLDialog::slotOk);
    this->layout()->addWidget(buttonBox);
}

void HTMLDialog::createContentPage()
{
    QWidget* contentPage = new QWidget;
    KPageWidgetItem* page = new KPageWidgetItem( contentPage, i18n("Content" ) );
    page->setHeader( i18n("Content" ) );
    page->setIcon( QIcon::fromTheme( QString::fromLatin1( "document-properties" ) ) );
    addPage( page );

    QVBoxLayout* lay1 = new QVBoxLayout( contentPage );
    QGridLayout* lay2 = new QGridLayout;
    lay1->addLayout( lay2 );

    QLabel* label = new QLabel( i18n("Page title:"), contentPage );
    lay2->addWidget( label, 0, 0 );
    m_title = new QLineEdit( contentPage );
    label->setBuddy( m_title );
    lay2->addWidget( m_title, 0, 1 );

    // Copyright
    label = new QLabel( i18n("Copyright:"), contentPage );
    label->setAlignment( Qt::AlignTop );
    lay2->addWidget( label, 1, 0 );
    m_copyright = new QLineEdit( contentPage );
    m_copyright->setText( Settings::SettingsData::instance()->HTMLCopyright() );
    label->setBuddy( m_copyright );
    lay2->addWidget( m_copyright, 1, 1 );

    // Description
    label = new QLabel( i18n("Description:"), contentPage );
    label->setAlignment( Qt::AlignTop );
    lay2->addWidget( label, 2, 0 );
    m_description = new KTextEdit( contentPage );
    label->setBuddy( m_description );
    lay2->addWidget( m_description, 2, 1 );

    m_generateKimFile = new QCheckBox( i18n("Create .kim export file"), contentPage );
    m_generateKimFile->setChecked( Settings::SettingsData::instance()->HTMLKimFile() );
    lay1->addWidget( m_generateKimFile );

    m_inlineMovies = new QCheckBox( i18nc("Inline as a verb, i.e. 'please show movies right on the page, not as links'",
                                          "Inline Movies in pages" ), contentPage );
    m_inlineMovies->setChecked( Settings::SettingsData::instance()->HTMLInlineMovies() );
    lay1->addWidget( m_inlineMovies );

    m_html5Video = new QCheckBox( i18nc("Tag as in HTML-tag, not as in image tag", "Use HTML5 video tag" ), contentPage );
    m_html5Video->setChecked( Settings::SettingsData::instance()->HTML5Video() );
    lay1->addWidget( m_html5Video );

    QString avconv = QStandardPaths::findExecutable(QString::fromUtf8("avconv"));
    const QString ffmpeg2theora = QStandardPaths::findExecutable(QString::fromUtf8("ffmpeg2theora"));

    QStandardPaths::findExecutable(QString::fromUtf8("avconv"));
    if ( avconv.isNull() )
        avconv = QStandardPaths::findExecutable(QString::fromUtf8("ffmpeg"));

    QString txt = i18n( "<p>This selection will generate video files suitable for displaying on web. "
                        "avconv and ffmpeg2theora are required for video file generation.</p>" );
    m_html5VideoGenerate = new QCheckBox( i18n( "Generate HTML5 video files (mp4 and ogg)" ), contentPage );
    m_html5VideoGenerate->setChecked( Settings::SettingsData::instance()->HTML5VideoGenerate() );
    lay1->addWidget( m_html5VideoGenerate );
    m_html5VideoGenerate->setWhatsThis( txt );
    if ( avconv.isNull() || ffmpeg2theora.isNull() )
        m_html5VideoGenerate->setEnabled( false );

    // What to include
    QGroupBox* whatToInclude = new QGroupBox( i18n( "What to Include" ), contentPage );
    lay1->addWidget( whatToInclude );
    QGridLayout* lay3 = new QGridLayout( whatToInclude );

    QCheckBox* cb = new QCheckBox( i18n("Description"), whatToInclude );
    m_whatToIncludeMap.insert( QString::fromLatin1("**DESCRIPTION**"), cb );
    lay3->addWidget( cb, 0, 0 );

    m_date = new QCheckBox( i18n("Date"), whatToInclude );
    m_date->setChecked( Settings::SettingsData::instance()->HTMLDate() );
    m_whatToIncludeMap.insert( QString::fromLatin1("**DATE**"), m_date );
    lay3->addWidget( m_date, 0, 1 );

    int row=1;
    int col=0;
    QString selectionsTmp = Settings::SettingsData::instance()->HTMLIncludeSelections();
    QStringMatcher* pattern = new QStringMatcher();
    pattern->setPattern(QString::fromLatin1("**DESCRIPTION**"));
    cb->setChecked( pattern->indexIn (selectionsTmp)  >= 0 ? 1 : 0 );

    QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    for( QList<DB::CategoryPtr>::Iterator it = categories.begin(); it != categories.end(); ++it ) {
        if ( ! (*it)->isSpecialCategory() ) {
            QCheckBox* cb = new QCheckBox((*it)->name(), whatToInclude);
            lay3->addWidget( cb, row, col%2 );
            m_whatToIncludeMap.insert( (*it)->name(), cb );
            pattern->setPattern((*it)->name());
            cb->setChecked( pattern->indexIn (selectionsTmp)  >= 0 ? 1 : 0 );
            if ( ++col % 2 == 0 )
                ++row;
        }
    }
}

void HTMLDialog::createLayoutPage()
{
    QWidget* layoutPage = new QWidget;
    KPageWidgetItem* page = new KPageWidgetItem( layoutPage, i18n("Layout" ) );
    page->setHeader( i18n("Layout" ) );
    page->setIcon( QIcon::fromTheme( QString::fromLatin1( "configure" )) );
    addPage(page);

    QVBoxLayout* lay1 = new QVBoxLayout( layoutPage );
    QGridLayout* lay2 = new QGridLayout;
    lay1->addLayout( lay2 );

    // Thumbnail size
    QLabel* label = new QLabel( i18n("Thumbnail size:"), layoutPage );
    lay2->addWidget( label, 0, 0 );

    QHBoxLayout* lay3 = new QHBoxLayout;
    lay2->addLayout( lay3, 0, 1 );

    m_thumbSize = new QSpinBox;
    m_thumbSize->setRange( 16, 256 );

    m_thumbSize->setValue( Settings::SettingsData::instance()->HTMLThumbSize() );
    lay3->addWidget( m_thumbSize );
    lay3->addStretch(1);
    label->setBuddy( m_thumbSize );

    // Number of columns
    label = new QLabel( i18n("Number of columns:"), layoutPage );
    lay2->addWidget( label, 1, 0 );

    QHBoxLayout* lay4 = new QHBoxLayout;
    lay2->addLayout( lay4, 1, 1 );
    m_numOfCols = new QSpinBox;
    m_numOfCols->setRange( 1, 10 );

    label->setBuddy( m_numOfCols);

    m_numOfCols->setValue( Settings::SettingsData::instance()->HTMLNumOfCols() );
    lay4->addWidget( m_numOfCols );
    lay4->addStretch( 1 );

    // Theme box
    label = new QLabel( i18n("Theme:"), layoutPage );
    lay2->addWidget( label, 2, 0 );
    lay4 = new QHBoxLayout;
    lay2->addLayout( lay4, 2, 1 );
    m_themeBox = new KComboBox( layoutPage );
    label->setBuddy( m_themeBox );
    lay4->addWidget( m_themeBox );
    lay4->addStretch( 1 );
    m_themeInfo = new QLabel( i18n("Theme Description"), layoutPage );
    m_themeInfo->setWordWrap(true);
    lay2->addWidget( m_themeInfo, 3, 1 );
    connect(m_themeBox, static_cast<void (KComboBox::*)(int)>(&KComboBox::currentIndexChanged), this, &HTMLDialog::displayThemeDescription);
    populateThemesCombo();

    // Image sizes
    QGroupBox* sizes = new QGroupBox( i18n("Image Sizes"), layoutPage );
    lay1->addWidget( sizes );
    QGridLayout* lay5 = new QGridLayout( sizes );
    ImageSizeCheckBox* size320  = new ImageSizeCheckBox( 320, 200, sizes );
    ImageSizeCheckBox* size640  = new ImageSizeCheckBox( 640, 480, sizes );
    ImageSizeCheckBox* size800  = new ImageSizeCheckBox( 800, 600, sizes );
    ImageSizeCheckBox* size1024 = new ImageSizeCheckBox( 1024, 768, sizes );
    ImageSizeCheckBox* size1280 = new ImageSizeCheckBox( 1280, 1024, sizes );
    ImageSizeCheckBox* size1600 = new ImageSizeCheckBox( 1600, 1200, sizes );
    ImageSizeCheckBox* sizeOrig = new ImageSizeCheckBox( i18n("Full size"), sizes );

    {
        int row = 0;
        int col = -1;
        lay5->addWidget( size320, row, ++col );
        lay5->addWidget( size640, row, ++col );
        lay5->addWidget( size800, row, ++col );
        lay5->addWidget( size1024, row, ++col );
        col =-1;
        lay5->addWidget( size1280, ++row, ++col );
        lay5->addWidget( size1600, row, ++col );
        lay5->addWidget( sizeOrig, row, ++col );
    }

    QString tmp;
    if ((tmp = Settings::SettingsData::instance()->HTMLSizes()) != QString::fromLatin1("")) {
        QStringMatcher* pattern = new QStringMatcher(QString::fromLatin1("320"));
        size320->setChecked( pattern->indexIn (tmp) >= 0 ? 1 : 0);
        pattern->setPattern(QString::fromLatin1("640"));
        size640->setChecked( pattern->indexIn (tmp) >= 0 ? 1 : 0);
        pattern->setPattern(QString::fromLatin1("800"));
        size800->setChecked( pattern->indexIn (tmp)  >= 0 ? 1 : 0 );
        pattern->setPattern(QString::fromLatin1("1024"));
        size1024->setChecked( pattern->indexIn (tmp)  >= 0 ? 1 : 0);
        pattern->setPattern(QString::fromLatin1("1280"));
        size1280->setChecked( pattern->indexIn (tmp)  >= 0 ? 1 : 0);
        pattern->setPattern(QString::fromLatin1("1600"));
        size1600->setChecked( pattern->indexIn (tmp)  >= 0 ? 1 : 0);
        pattern->setPattern(QString::fromLatin1("-1"));
        sizeOrig->setChecked( pattern->indexIn (tmp)  >= 0 ? 1 : 0);
    } else
        size800->setChecked( 1 );

    m_sizeCheckBoxes << size800 << size1024 << size1280 << size640 << size1600 << size320 << sizeOrig;

    lay1->addStretch(1);
    QGridLayout* lay6 = new QGridLayout;
    lay1->addLayout( lay6 );
}

void HTMLDialog::createDestinationPage()
{
    QWidget* destinationPage = new QWidget;

    KPageWidgetItem* page = new KPageWidgetItem( destinationPage, i18n("Destination" ) );
    page->setHeader( i18n("Destination" ) );
    page->setIcon( QIcon::fromTheme( QString::fromLatin1( "drive-harddisk" ) ) );
    addPage( page );

    QVBoxLayout* lay1 = new QVBoxLayout( destinationPage );
    QGridLayout* lay2 = new QGridLayout;
    lay1->addLayout( lay2 );
    int row = -1;

    // Base Directory
    QLabel* label = new QLabel( i18n("Base directory:"), destinationPage );
    lay2->addWidget( label, ++row, 0 );
    QHBoxLayout* lay3 = new QHBoxLayout;
    lay2->addLayout( lay3, row, 1 );

    m_baseDir = new QLineEdit( destinationPage );
    lay3->addWidget( m_baseDir );
    label->setBuddy( m_baseDir );

    QPushButton* but = new QPushButton( QString::fromLatin1( ".." ), destinationPage );
    lay3->addWidget( but );
    but->setFixedWidth( 25 );

    connect(but, &QPushButton::clicked, this, &HTMLDialog::selectDir);
    m_baseDir->setText( Settings::SettingsData::instance()->HTMLBaseDir() );

    // Output Directory
    label = new QLabel( i18n("Gallery directory:"), destinationPage );
    lay2->addWidget( label, ++row, 0 );
    m_outputDir = new QLineEdit( destinationPage );
    lay2->addWidget( m_outputDir, row, 1 );
    label->setBuddy( m_outputDir );

    // fully "Assembled" output Directory
    label = new QLabel( i18n("Output directory:"), destinationPage );
    lay2->addWidget( label, ++row, 0 );
    m_outputLabel = new QLabel( destinationPage );
    lay2->addWidget( m_outputLabel, row, 1 );
    label->setBuddy( m_outputLabel );
    connect(m_baseDir, &QLineEdit::textChanged, this, &HTMLDialog::slotUpdateOutputLabel);
    connect(m_outputDir, &QLineEdit::textChanged, this, &HTMLDialog::slotUpdateOutputLabel);
    // initial text
    slotUpdateOutputLabel();

    // Destination URL
    label = new QLabel( i18n("URL for final destination of .kim file:" ), destinationPage );
    label->setToolTip( i18n(
                           "<p>If you move the gallery to a remote location, set this to the destination URL.</p>"
                           "<p>This only affects the generated <filename>.kim</filename> file.</p>"
                           )
                       );
    lay2->addWidget( label, ++row, 0 );
    m_destURL = new QLineEdit( destinationPage );
    m_destURL->setText( Settings::SettingsData::instance()->HTMLDestURL() );
    lay2->addWidget( m_destURL, row, 1 );
    label->setBuddy( m_destURL );

    // Base URL
    label = new QLabel( i18n("Open gallery in browser:"), destinationPage );
    lay2->addWidget( label, ++row, 0 );
    m_openInBrowser = new QCheckBox(destinationPage);
    m_openInBrowser->setChecked(true);
    lay2->addWidget( m_openInBrowser, row, 1);
    label->setBuddy( m_openInBrowser );

    lay1->addStretch( 1 );
}

void HTMLDialog::slotOk()
{
    if ( !checkVars() )
        return;

    if( activeResolutions().count() < 1 ) {
        KMessageBox::sorry( nullptr, i18n( "You must select at least one resolution." ) );
        return;
    }

    accept();

    Settings::SettingsData::instance()->setHTMLBaseDir( m_baseDir->text() );
    Settings::SettingsData::instance()->setHTMLDestURL( m_destURL->text() );
    Settings::SettingsData::instance()->setHTMLCopyright( m_copyright->text() );
    Settings::SettingsData::instance()->setHTMLDate( m_date->isChecked() );
    Settings::SettingsData::instance()->setHTMLTheme( m_themeBox->currentIndex() );
    Settings::SettingsData::instance()->setHTMLKimFile( m_generateKimFile->isChecked() );
    Settings::SettingsData::instance()->setHTMLInlineMovies( m_inlineMovies->isChecked() );
    Settings::SettingsData::instance()->setHTML5Video( m_html5Video->isChecked() );
    Settings::SettingsData::instance()->setHTML5VideoGenerate( m_html5VideoGenerate->isChecked() );
    Settings::SettingsData::instance()->setHTMLThumbSize( m_thumbSize->value() );
    Settings::SettingsData::instance()->setHTMLNumOfCols( m_numOfCols->value() );
    Settings::SettingsData::instance()->setHTMLSizes( activeSizes() );
    Settings::SettingsData::instance()->setHTMLIncludeSelections( includeSelections() );

    Generator generator( setup(), this );
    generator.generate();
}

void HTMLDialog::selectDir()
{
    QString dir = QFileDialog::getExistingDirectory( this,
                                                     i18n("Select base directory..."),
                                                     m_baseDir->text() );
    if ( !dir.isEmpty() )
        m_baseDir->setText( dir );
}

bool HTMLDialog::checkVars()
{
    QString outputDir = m_baseDir->text() + QString::fromLatin1( "/" ) + m_outputDir->text();


    // Ensure base dir is specified
    QString baseDir = m_baseDir->text();
    if ( baseDir.isEmpty() ) {
        KMessageBox::error( this, i18n("<p>You did not specify a base directory. "
                                       "This is the topmost directory for your images. "
                                       "Under this directory you will find each generated collection "
                                       "in separate directories.</p>"),
                            i18n("No Base Directory Specified") );
        return false;
    }

    // ensure output directory is specified
    if ( m_outputDir->text().isEmpty() ) {
        KMessageBox::error( this, i18n("<p>You did not specify an output directory. "
                                       "This is a directory containing the actual images. "
                                       "The directory will be in the base directory specified above.</p>"),
                            i18n("No Output Directory Specified") );
        return false;
    }

    // ensure base dir exists
    QScopedPointer<KIO::StatJob> statJob( KIO::stat( QUrl::fromUserInput(baseDir), KIO::StatJob::DestinationSide, 1 /*only basic info*/));
    KJobWidgets::setWindow( statJob.data(), MainWindow::Window::theMainWindow() );
    if (!statJob->exec())
    {
        KMessageBox::error( this, i18n("<p>Error while reading information about %1. "
                                       "This is most likely because the directory does not exist.</p>"
                                       "<p>The error message was: %2</p>",
                                       baseDir, statJob->errorString() ) );
        return false;
    }

    KFileItem fileInfo( statJob->statResult(), QUrl::fromUserInput(baseDir) );
    if ( !fileInfo.isDir() ) {
        KMessageBox::error( this, i18n("<p>%1 does not exist, is not a directory or "
                                       "cannot be written to.</p>", baseDir ) );
        return false;
    }


    // test if destination directory exists.
    QScopedPointer<KIO::StatJob> existsJob( KIO::stat( QUrl::fromUserInput(outputDir), KIO::StatJob::DestinationSide, 0 /*only minimal info*/ ));
    KJobWidgets::setWindow( existsJob.data(), MainWindow::Window::theMainWindow() );
    if ( existsJob->exec() )
    {
        int answer = KMessageBox::warningYesNo( this,
                                                i18n("<p>Output directory %1 already exists. "
                                                     "Usually, this means you should specify a new directory.</p>"
                                                     "<p>Should %2 be deleted first?</p>", outputDir, outputDir ),
                                                i18n("Directory Exists"), KStandardGuiItem::yes(), KStandardGuiItem::no(),
                                                QString::fromLatin1("html_export_delete_original_directory") );
        if ( answer == KMessageBox::Yes ) {
            QScopedPointer<KJob> delJob (KIO::del(QUrl::fromUserInput(outputDir)));
            KJobWidgets::setWindow( delJob.data(), MainWindow::Window::theMainWindow() );
            delJob->exec();
        }
        else
            return false;
    }
    return true;
}

QList<ImageSizeCheckBox*> HTMLDialog::activeResolutions() const
{
    QList<ImageSizeCheckBox*> res;
    for( QList<ImageSizeCheckBox*>::ConstIterator sizeIt = m_sizeCheckBoxes.begin(); sizeIt != m_sizeCheckBoxes.end(); ++sizeIt ) {
        if ( (*sizeIt)->isChecked() )
            res << *sizeIt;
    }
    return res;
}

QString HTMLDialog::activeSizes() const
{
    QString res;
    for( QList<ImageSizeCheckBox*>::ConstIterator sizeIt = m_sizeCheckBoxes.begin(); sizeIt != m_sizeCheckBoxes.end(); ++sizeIt ) {
        if ( (*sizeIt)->isChecked() ) {
            if (res.length() > 0)
                res.append(QString::fromLatin1(","));
            res.append(QString::number((*sizeIt)->width()));
        }
    }
    return res;
}

QString HTMLDialog::includeSelections() const
{
    QString sel;
    Setup setupChoices = setup();

    for( QMap<QString,QCheckBox*>::ConstIterator it = m_whatToIncludeMap.begin()
         ; it != m_whatToIncludeMap.end()
         ; ++it )
    {
        QString name = it.key();
        if ( setupChoices.includeCategory(name) ) {
            if (sel.length() > 0)
                sel.append(QString::fromLatin1(","));
            sel.append(name);
        }
    }
    return sel;
}

void HTMLDialog::populateThemesCombo()
{
    QStringList dirs = QStandardPaths::locateAll(
                QStandardPaths::GenericDataLocation,
                QString::fromLocal8Bit("kphotoalbum/themes/"),
                QStandardPaths::LocateDirectory
                );
    int i = 0;
    int theme = 0;
    int defaultthemes = 0;
    Debug() << "Theme directories:"<<dirs;
    for(QStringList::Iterator it = dirs.begin(); it != dirs.end(); ++it) {
        QDir dir(*it);
        Debug() << "Searching themes in:"<<dir;
        QStringList themes = dir.entryList( QDir::Dirs | QDir::Readable );
        for(QStringList::Iterator it = themes.begin(); it != themes.end(); ++it) {
            Debug() << " *" << *it;
            if(*it == QString::fromLatin1(".") || *it == QString::fromLatin1("..")) continue;
            QString themePath = QString::fromLatin1("%1/%2/").arg(dir.path()).arg(*it);

            KConfig themeconfig( QString::fromLatin1( "%1/kphotoalbum.theme").arg( themePath ), KConfig::SimpleConfig );
            KConfigGroup config = themeconfig.group("theme");
            QString themeName = config.readEntry( "Name" );
            QString themeAuthor = config.readEntry( "Author" );
            m_themeAuthors << themeAuthor; // save author to display later
            QString themeDefault = config.readEntry( "Default" );
            QString themeDescription = config.readEntry( "Description" );
            m_themeDescriptions << themeDescription; // save description to display later

            //m_themeBox->insertItem( i, i18n( "%1 (by %2)",themeName, themeAuthor ) ); // combined alternative
            m_themeBox->insertItem( i, i18n( "%1",themeName) );
            m_themes.insert( i, themePath );

            if (themeDefault == QString::fromLatin1("true")) {
                theme = i;
                defaultthemes++;
            }
            i++;
        }
    }
    if(m_themeBox->count() < 1) {
        KMessageBox::error( this, i18n("Could not find any themes - this is very likely an installation error" ) );
    }
    if ( (Settings::SettingsData::instance()->HTMLTheme() >= 0) && (Settings::SettingsData::instance()->HTMLTheme() < m_themeBox->count()) )
        m_themeBox->setCurrentIndex( Settings::SettingsData::instance()->HTMLTheme() );
    else {
        m_themeBox->setCurrentIndex( theme );
        if (defaultthemes > 1)
            KMessageBox::information( this, i18n("More than one theme is set as default, using theme %1", m_themeBox->currentText()) );
    }
}

void HTMLDialog::displayThemeDescription(int themenr)
{
    // SLOT: update m_themeInfo label whenever the m_theme QComboBox changes.
    QString outtxt = i18nc( "This is to show the author of the theme. E.g. copyright character (&#169;) by itself will work fine on this context if no proper word is available in your language.", "by " );
    outtxt.append( m_themeAuthors[themenr] );
    outtxt.append( i18n( "\n " ) );
    outtxt.append( m_themeDescriptions[themenr] );
    m_themeInfo->setText( outtxt );
    // Instead of two separate lists for authors and descriptions one could have a combined one by appending the text prior to storing within populateThemesCombo(),
    // however, storing author and descriptions separately might be cleaner.
}

void HTMLDialog::slotUpdateOutputLabel()
{
    QString outputDir = QDir(m_baseDir->text()).filePath(m_outputDir->text());
    // feedback on validity:
    if (outputDir == m_baseDir->text())
    {
        m_outputLabel->setStyleSheet(QString::fromLatin1("QLabel { color : darkred; }"));
        outputDir.append(i18n("<p>Gallery directory cannot be empty.</p>"));
    } else if ( QDir(outputDir).exists())
    {
        m_outputLabel->setStyleSheet(QString::fromLatin1("QLabel { color : darkorange; }"));
        outputDir.append(i18n("<p>The output directory already exists.</p>"));
    } else
    {
        m_outputLabel->setStyleSheet(QString::fromLatin1("QLabel { color : black; }"));
    }
    m_outputLabel->setText( outputDir );

}

void HTMLDialog::slotSuggestOutputDir()
{
    if (m_outputDir->text().isEmpty())
    {
        // the title is often an adequate directory name:
        m_outputDir->setText( m_title->text() );
    }
}

int HTMLDialog::exec(const DB::FileNameList& list)
{
    if (list.empty())
    {
        qWarning() << "HTMLDialog called without images for export";
        return false;
    }
    m_list = list;
    return QDialog::exec();
}



Setup HTMLGenerator::HTMLDialog::setup() const
{
    Setup setup;
    setup.setTitle( m_title->text() );
    setup.setBaseDir( m_baseDir->text() );
    if (m_openInBrowser->isEnabled())
    {
        setup.setBaseURL( m_baseDir->text() );
    }
    setup.setDestURL( m_destURL->text() );
    setup.setOutputDir( m_outputDir->text() );
    setup.setThumbSize( m_thumbSize->value() );
    setup.setCopyright( m_copyright->text() );
    setup.setDate( m_date->isChecked() );
    setup.setDescription( m_description->toPlainText() );
    setup.setNumOfCols( m_numOfCols->value() );
    setup.setGenerateKimFile( m_generateKimFile->isChecked() );
    setup.setThemePath( m_themes[m_themeBox->currentIndex()] );
    for( QMap<QString,QCheckBox*>::ConstIterator includeIt = m_whatToIncludeMap.begin();
         includeIt != m_whatToIncludeMap.end(); ++includeIt ) {
        setup.setIncludeCategory( includeIt.key(), includeIt.value()->isChecked() );
    }
    setup.setImageList(m_list);

    setup.setResolutions( activeResolutions() );
    setup.setInlineMovies( m_inlineMovies->isChecked() );
    setup.setHtml5Video( m_html5Video->isChecked() );
    setup.setHtml5VideoGenerate( m_html5VideoGenerate->isChecked() );
    return setup;
}

#include "HTMLDialog.moc"
// vi:expandtab:tabstop=4 shiftwidth=4:
