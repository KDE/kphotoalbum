#include "FileVersionDetectionPage.h"
#include "SettingsData.h"
#include <klocale.h>
#include <KComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <Q3VGroupBox>
#include <QCheckBox>
#include <QLineEdit>

Settings::FileVersionDetectionPage::FileVersionDetectionPage( QWidget* parent )
    : QWidget( parent )
{
    QVBoxLayout* lay1 = new QVBoxLayout( this );

    // Original/Modified File Support
    Q3VGroupBox* modifiedBox = new Q3VGroupBox( i18n("File Version Detection Settings"), this );
    lay1->addWidget( modifiedBox );

    _detectModifiedFiles = new QCheckBox(i18n("Try to detect multiple versions of files"), modifiedBox);

    QLabel* modifiedFileComponentLabel = new QLabel( i18n("File versions search regexp:" ), modifiedBox );
    _modifiedFileComponent = new QLineEdit(modifiedBox);

    QLabel* originalFileComponentLabel = new QLabel( i18n("Original file replacement text:" ), modifiedBox );
    _originalFileComponent = new QLineEdit(modifiedBox);

    _moveOriginalContents = new QCheckBox(i18n("Move meta-data (i.e. delete tags from the original):"), modifiedBox);

    _autoStackNewFiles = new QCheckBox(i18n("Auto-stack new files on top of old"), modifiedBox);

    // Copy File Support
    Q3VGroupBox* copyBox = new Q3VGroupBox( i18nc("Configure the feature to make a copy of a file first and then open the copied file with an external applicaiton", "Copy File and Open on External Application"), this );
    lay1->addWidget( copyBox );

    QLabel* copyFileComponentLabel = new QLabel( i18n("Copy file search regexp:" ), copyBox );
    _copyFileComponent = new QLineEdit(copyBox);

    QLabel* copyFileReplacementComponentLabel = new QLabel( i18n("Copy file replacement text:" ), copyBox );
    _copyFileReplacementComponent = new QLineEdit(copyBox);

    QString txt;
    txt = i18n( "<p>When KPhotoAlbum searches for new files and finds a file that matches the <i>modified file search regexp</i> it is assumed that an original version of the image may exist.  The regexp pattern will be replaced with the <i>original file string</i> text and if that file exists, all associated metadata (category information, ratings, etc) will be copied from the original file to the new one.</p>");
    _detectModifiedFiles->setWhatsThis( txt );
    modifiedFileComponentLabel->setWhatsThis( txt );
    _modifiedFileComponent->setWhatsThis( txt );
    originalFileComponentLabel->setWhatsThis( txt );
    _originalFileComponent->setWhatsThis( txt );
    _moveOriginalContents->setWhatsThis( txt );
    _autoStackNewFiles->setWhatsThis( txt );

    txt = i18n("<p>KPhotoAlbum can make a copy of an image before opening it with an external program.  These settings set the original regexp to search for and contents to replace it with when deciding what the new filename should be.</p>");
    copyFileComponentLabel->setWhatsThis( txt );
    _copyFileComponent->setWhatsThis( txt );
    copyFileReplacementComponentLabel->setWhatsThis( txt );
    _copyFileReplacementComponent->setWhatsThis( txt );
}

void Settings::FileVersionDetectionPage::loadSettings( Settings::SettingsData* opt )
{
    _detectModifiedFiles->setChecked( opt->detectModifiedFiles() );
    _modifiedFileComponent->setText( opt->modifiedFileComponent() );
    _originalFileComponent->setText( opt->originalFileComponent() );
    _moveOriginalContents->setChecked( opt->moveOriginalContents() );
    _autoStackNewFiles->setChecked( opt->autoStackNewFiles() );
    _copyFileComponent->setText( opt->copyFileComponent() );
    _copyFileReplacementComponent->setText( opt->copyFileReplacementComponent() );
}

void Settings::FileVersionDetectionPage::saveSettings( Settings::SettingsData* opt )
{
    opt->setDetectModifiedFiles( _detectModifiedFiles->isChecked() );
    opt->setModifiedFileComponent( _modifiedFileComponent->text() );
    opt->setOriginalFileComponent( _originalFileComponent->text() );
    opt->setAutoStackNewFiles( _autoStackNewFiles->isChecked() );
    opt->setCopyFileComponent( _copyFileComponent->text() );
    opt->setCopyFileReplacementComponent( _copyFileReplacementComponent->text() );
}
