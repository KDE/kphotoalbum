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
#include "DatabaseBackendPage.h"
#include <Q3ButtonGroup>
#include <klocale.h>
#include "SettingsData.h"
#include <QSpacerItem>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QLabel>
#include <QCheckBox>
#include <Q3VGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>
#include "config-kpa-sqldb.h"
#include "MainWindow/DirtyIndicator.h"

#ifdef SQLDB_SUPPORT
#  include <SQLDB/SQLSettingsWidget.h>
#  include <SQLDB/DatabaseAddress.h>
#endif

Settings::DatabaseBackendPage::DatabaseBackendPage( QWidget* parent )
    :QWidget( parent )
{
    QVBoxLayout* lay1 = new QVBoxLayout(this);

    _backendButtons = new Q3ButtonGroup(1, Qt::Horizontal,
                                        i18n("Database backend to use"), this);
    lay1->addWidget(_backendButtons);

    new QRadioButton(i18n("XML backend (recommended)"), _backendButtons);
#ifdef SQLDB_SUPPORT
    //QRadioButton* sqlButton =
    new QRadioButton(i18n("SQL backend (experimental)"), _backendButtons);
#endif

    // XML Backend
    Q3VGroupBox* xmlBox = new Q3VGroupBox( i18n("XML Database Setting"), this );
    lay1->addWidget( xmlBox );

    // Compressed index.xml
    _compressedIndexXML = new QCheckBox( i18n("Choose speed over readability for index.xml file"), xmlBox );
    connect( _compressedIndexXML, SIGNAL( clicked(bool) ), this, SLOT ( markDirty() ) );

    _compressBackup = new QCheckBox( i18n( "Compress backup file" ), xmlBox );

    // Auto save
    QWidget* box = new QWidget( xmlBox );
    QLabel* label = new QLabel( i18n("Auto save every:"), box );
    _autosave = new QSpinBox;
    _autosave->setRange( 1, 120 );
    _autosave->setSuffix( i18n( "min." ) );

    QHBoxLayout* lay = new QHBoxLayout( box );
    lay->addWidget( label );
    lay->addWidget( _autosave );
    lay->addStretch( 1 );

    // Backup
    box = new QWidget( xmlBox );
    lay = new QHBoxLayout( box );
    QLabel* backupLabel = new QLabel( i18n( "Number of backups to keep:" ), box );
    lay->addWidget( backupLabel );

    _backupCount = new QSpinBox;
    _backupCount->setRange( -1, 100 );
    _backupCount->setSpecialValueText( i18n( "Infinite" ) );
    lay->addWidget( _backupCount );
    lay->addStretch( 1 );

    QString txt;
    txt = i18n("<p>KPhotoAlbum is capable of backing up the index.xml file by keeping copies named index.xml~1~ index.xml~2~ etc. "
               "and you can use the spinbox to specify the number of backup files to keep. "
               "KPhotoAlbum will delete the oldest backup file when it reaches "
               "the maximum number of backup files.</p>"
               "<p>The index.xml file may grow substantially if you have many images, and in that case it is useful to ask KPhotoAlbum to zip "
               "the backup files to preserve disk space.</p>" );
    backupLabel->setWhatsThis( txt );
    _backupCount->setWhatsThis( txt );
    _compressBackup->setWhatsThis( txt );

    txt = i18n( "<p>KPhotoAlbum is using a single index.xml file as its <i>data base</i>. With lots of images it may take "
                "a long time to read this file. You may cut down this time to approximately half, by checking this check box. "
                "The disadvantage is that the index.xml file is less readable by human eyes.</p>");
    _compressedIndexXML->setWhatsThis( txt );

    // SQL Backend
#ifdef SQLDB_SUPPORT
    Q3VGroupBox* sqlBox = new Q3VGroupBox(i18n("SQL Database Settings"), this);
    //sqlBox->setEnabled(false);
    lay1->addWidget(sqlBox);

    _sqlSettings = new SQLDB::SQLSettingsWidget(sqlBox);

    QLabel* passwordWarning =
        new QLabel(i18n("Warning: The password is saved as plain text to the configuration file."), this);
    passwordWarning->hide();
    lay1->addWidget(passwordWarning);

    QSpacerItem* spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    lay1->addItem(spacer);

    //connect(sqlButton, SIGNAL(toggled(bool)), sqlBox, SLOT(setEnabled(bool)));
    connect(_sqlSettings, SIGNAL(passwordChanged(const QString&)), passwordWarning, SLOT(show()));
#endif /* SQLDB_SUPPORT */
}

void Settings::DatabaseBackendPage::loadSettings( Settings::SettingsData* opt )
{
    _compressedIndexXML->setChecked( opt->useCompressedIndexXML() );
    _autosave->setValue( opt->autoSave() );
    _backupCount->setValue( opt->backupCount() );
    _compressBackup->setChecked( opt->compressBackup() );

    const QString backend = Settings::SettingsData::instance()->backend();
    if (backend == QString::fromLatin1("xml"))
        _backendButtons->setButton(0);
#ifdef SQLDB_SUPPORT
    else if (backend == QString::fromLatin1("sql"))
        _backendButtons->setButton(1);

    _sqlSettings->setSettings(Settings::SettingsData::instance()->SQLParameters());
#endif
}

void Settings::DatabaseBackendPage::saveSettings( Settings::SettingsData* opt )
{
    const char* backendNames[] = { "xml", "sql" };
    int backendIndex = _backendButtons->selectedId();
    if (backendIndex < 0 || backendIndex >= 2)
        backendIndex = 0;
    opt->setBackend(QString::fromLatin1(backendNames[backendIndex]));

    opt->setBackupCount( _backupCount->value() );
    opt->setCompressBackup( _compressBackup->isChecked() );
    opt->setUseCompressedIndexXML( _compressedIndexXML->isChecked() );
    opt->setAutoSave( _autosave->value() );

    // SQLDB
#ifdef SQLDB_SUPPORT
    if (_sqlSettings->hasSettings())
        opt->setSQLParameters(_sqlSettings->getSettings());
#endif


}

void Settings::DatabaseBackendPage::markDirty()
{
    MainWindow::DirtyIndicator::markDirty();
}
