#include "import.h"
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
#include "options.h"
#include "importmatcher.h"
#include <qcheckbox.h>
#include <qcombobox.h>
#include "util.h"
#include "imagedb.h"
class KPushButton;

void Import::imageImport()
{
    QString file = KFileDialog::getOpenFileName( QString::null, QString::fromLatin1( "*.kim|KimDaBa export files" ), 0 );
    if ( file.isNull() )
        return;

    bool ok;
    Import dialog( file, &ok, 0, "import_dialog" );
    dialog.resize( 800, 600 );
    if ( ok )
        dialog.exec();
}

Import::Import( const QString& fileName, bool* ok, QWidget* parent, const char* name )
    :KWizard( parent, name ), _zipFile( fileName )
{
    KZip zip( fileName );
    if ( !zip.open( IO_ReadOnly ) ) {
        KMessageBox::error( this, i18n("Unable to open '%1' for reading").arg( fileName ), i18n("Error importing data") );
        *ok = false;
        return;
    }
    const KArchiveDirectory* dir = zip.directory();
    if ( dir == 0 ) {
        KMessageBox::error( this, i18n( "Error reading directory contest of file %1. The file is likely broken" ).arg( fileName ) );
        *ok = false;
        return;
    }

    const KArchiveEntry* indexxml = dir->entry( QString::fromLatin1( "index.xml" ) );
    if ( indexxml == 0 || ! indexxml->isFile() ) {
        KMessageBox::error( this, i18n( "Error reading index.xml file from %1. The file is likely broken" ).arg( fileName ) );
        *ok = false;
        return;
    }

    const KArchiveFile* file = static_cast<const KArchiveFile*>( indexxml );
    QByteArray data = file->data();

    *ok = readFile( data, fileName );
    if ( !*ok ) {
        return;
    }

    setupPages();
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
    if ( ! (top.tagName().lower() == QString::fromLatin1( "kimdaba-export" ) ) ) {
        KMessageBox::error( this, i18n("Unexpected top element while reading file %1. Expected KimDaBa-export found %2")
                            .arg( fileName ).arg( top.tagName() ) );
        return false;
    }

    for ( QDomNode node = top.firstChild(); !node.isNull(); node = node.nextSibling() ) {
        if ( !node.isElement() || ! (node.toElement().tagName().lower() == QString::fromLatin1( "image" ) ) ) {
            KMessageBox::error( this, i18n("Unknow element while reading %1, expected Image").arg( fileName ) );
            return false;
        }
        QDomElement elm = node.toElement();

        ImageInfo* info = new ImageInfo( elm.attribute( QString::fromLatin1( "file" ) ), elm );
        _images.append( info );
    }

    return true;
}

void Import::setupPages()
{
    helpButton()->hide();

    createIntroduction();
    createDestination();
    createOptionPages();
    connect( this, SIGNAL( selected( const QString& ) ), this, SLOT( updateNextButtonState() ) );
    connect( finishButton(), SIGNAL( clicked() ), this, SLOT( slotFinish() ) );
}

void Import::createIntroduction()
{
    QString txt = i18n( "<qt><p>This wizard will take you through the steps of an import operation</p></qt>" );
    QLabel* intro = new QLabel( txt, this );
    addPage( intro, i18n("Introduction") );
}

void Import::createDestination()
{
    QWidget* top = new QWidget( this );
    _destinationPage = top;
    QVBoxLayout* topLay = new QVBoxLayout( top, 6 );
    topLay->addStretch( 1 );
    QHBoxLayout* lay = new QHBoxLayout( topLay, 6 );
    topLay->addStretch( 1 );

    QLabel* label = new QLabel( i18n( "Destination of images: " ), top );
    lay->addWidget( label );

    _destinationEdit = new KLineEdit( top );
    lay->addWidget( _destinationEdit, 1 );

    KPushButton* but = new KPushButton( QString::fromLatin1("..." ), top );
    but->setFixedWidth( 30 );
    lay->addWidget( but );


    _destinationEdit->setText( Options::instance()->imageDirectory());
    connect( but, SIGNAL( clicked() ), this, SLOT( slotEditDestination() ) );
    connect( _destinationEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( updateNextButtonState() ) );
    addPage( top, i18n("Destination of images" ) );
}

void  Import::slotEditDestination()
{
    QString file = KFileDialog::getExistingDirectory( _destinationEdit->text(), this );
    if ( !file.isNull() ) {
        if ( ! QFileInfo(file).absFilePath().startsWith( QFileInfo(Options::instance()->imageDirectory()).absFilePath()) ) {
            KMessageBox::error( this, i18n("The directory must be a subdirectory of %1").arg( Options::instance()->imageDirectory() ) );
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
        if ( !QFileInfo( dest ).isDir() )
            enabled = false;
        else if ( ! QFileInfo(dest).absFilePath().startsWith( QFileInfo(Options::instance()->imageDirectory()).absFilePath()) )
            enabled = false;
    }

    nextButton()->setEnabled( enabled );
}

void Import::createOptionPages()
{
    QStringList options;
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        ImageInfo* info = *it;
        QStringList opts = info->availableOptionGroups();
        for( QStringList::Iterator optsIt = opts.begin(); optsIt != opts.end(); ++optsIt ) {
            if ( !options.contains( *optsIt ) )
                options.append( *optsIt );
        }
    }

    _optionGroupMatcher = new ImportMatcher( QString::null, QString::null, options, Options::instance()->optionGroups(),
                                                this, "import matcher" );
    addPage( _optionGroupMatcher, i18n("Match option groups") );

    _dummy = new QWidget( this );
    addPage( _dummy, QString::null );
}

ImportMatcher* Import::createOptionPage( const QString& myOptionGroup, const QString& otherOptionGroup )
{
    QStringList otherOptions;
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        ImageInfo* info = *it;
        QStringList opts = info->optionValue( otherOptionGroup );
        for( QStringList::Iterator optsIt = opts.begin(); optsIt != opts.end(); ++optsIt ) {
            if ( !otherOptions.contains( *optsIt ) )
                otherOptions.append( *optsIt );
        }
    }

    QStringList myOptions = Options::instance()->optionValueInclGroups( myOptionGroup );
    ImportMatcher* matcher = new ImportMatcher( otherOptionGroup, myOptionGroup, otherOptions, myOptions, this, "import matcher" );
    addPage( matcher, myOptionGroup );
    return matcher;
}

void Import::next()
{
    static bool hasFilled = false;
    if ( !hasFilled && currentPage() == _optionGroupMatcher ) {
        hasFilled = true;
        _optionGroupMatcher->setEnabled( false );
        delete _dummy;

        ImportMatcher* matcher = 0;
        for( QValueList<OptionMatch*>::Iterator it = _optionGroupMatcher->_matchers.begin();
             it != _optionGroupMatcher->_matchers.end();
             ++it )
        {
            OptionMatch* match = *it;
            if ( match->_checkbox->isChecked() ) {
                matcher = createOptionPage( match->_combobox->currentText(), match->_checkbox->text() );
                _matchers.append( matcher );
            }
        }
        if ( matcher )
            setFinishEnabled( matcher, true );
    }

    QWizard::next();
}

QMap<QString,QString> Import::copyFilesFromZipFile()
{
    QMap< QString, QString> map;

    KZip zip( _zipFile );
    if ( !zip.open( IO_ReadOnly ) ) {
        KMessageBox::error( this, i18n("Unable to open '%1' for reading").arg( _zipFile ), i18n("Error importing data") );
        return QMap<QString,QString>();
    }
    const KArchiveDirectory* dir = zip.directory();
    if ( dir == 0 ) {
        KMessageBox::error( this, i18n( "Error reading directory contest of file %1. The file is likely broken" ).arg( _zipFile ) );
        return QMap<QString,QString>();
    }

    QStringList files = dir->entries();
    files.sort();
    int index = 0;
    for( QStringList::Iterator fileIt = files.begin(); fileIt != files.end(); ++fileIt ) {
        if ( ! (*fileIt).startsWith( QString::fromLatin1("image") ) )
            continue;

        const KArchiveEntry* fileEntry = dir->entry( *fileIt );
        if ( fileEntry == 0 || !fileEntry->isFile() )
            continue;

        const KArchiveFile* file = static_cast<const KArchiveFile*>( fileEntry );
        QByteArray data = file->data();

        bool exists = true;
        QString newName;
        while ( exists ) {
            newName = QString::fromLatin1( "%1/image%2.%3" ).arg(_destinationEdit->text()).arg( Util::pad(6,++index) )
                      .arg( QFileInfo( *fileIt ).extension() );
            exists = QFileInfo( newName ).exists();
        }

        QString relativeName = newName.mid( Options::instance()->imageDirectory().length() );
        map.insert( *fileIt, relativeName );

        QFile out( newName );
        if ( !out.open( IO_WriteOnly ) ) {
            KMessageBox::error( this, i18n("Error when writing image %s").arg( newName ) );
            return QMap<QString,QString>();
        }
        out.writeBlock( data, data.size() );
        out.close();
    }

    return map;
}

void Import::slotFinish()
{
    QMap<QString,QString> map = copyFilesFromZipFile();
    if ( map.size() == 0 )
        return;

    // Run though all images
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        ImageInfo* info = *it;

        ImageInfo* newInfo = new ImageInfo( map[info->fileName(true)] );
        newInfo->setLabel( info->label() );
        newInfo->setDescription( info->description() );
        newInfo->setStartDate( info->startDate() );
        newInfo->setEndDate( info->endDate() );
        newInfo->rotate( info->angle() );
        newInfo->setDrawList( info->drawList() );
        newInfo->setMD5Sum( info->MD5Sum() );
        ImageDB::instance()->images().append( newInfo );

        // Run though the optionGroups
        for( QValueList<ImportMatcher*>::Iterator grpIt = _matchers.begin(); grpIt != _matchers.end(); ++grpIt ) {
            QString otherGrp = (*grpIt)->_otherOptionGroup;
            QString myGrp = (*grpIt)->_myOptionGroup;

            // Run through each option
            QValueList<OptionMatch*>& matcher = (*grpIt)->_matchers;
            for( QValueList<OptionMatch*>::Iterator optionIt = matcher.begin(); optionIt != matcher.end(); ++optionIt ) {
                if ( !(*optionIt)->_checkbox->isChecked() )
                    continue;
                QString otherOption = (*optionIt)->_checkbox->text();
                QString myOption = (*optionIt)->_combobox->currentText();

                if ( info->hasOption( otherGrp, otherOption ) ) {
                    newInfo->addOption( myGrp, myOption );
                }

            }
        }
    }
}


