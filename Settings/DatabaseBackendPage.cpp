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
#include <KLocalizedString>
#include "SettingsData.h"
#include <QHBoxLayout>
#include <QSpinBox>
#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>
#include "MainWindow/DirtyIndicator.h"

Settings::DatabaseBackendPage::DatabaseBackendPage( QWidget* parent )
    :QWidget( parent )
{
    QVBoxLayout* topLayout = new QVBoxLayout(this);

    // Compressed index.xml
    m_compressedIndexXML = new QCheckBox( i18n("Choose speed over readability for index.xml file"), this );
    topLayout->addWidget(m_compressedIndexXML);
    connect(m_compressedIndexXML, &QCheckBox::clicked, this, &DatabaseBackendPage::markDirty);

    m_compressBackup = new QCheckBox( i18n( "Compress backup file" ), this );
    topLayout->addWidget(m_compressBackup);

    // Auto save
    QLabel* label = new QLabel( i18n("Auto save every:"), this );
    m_autosave = new QSpinBox;
    m_autosave->setRange( 1, 120 );
    m_autosave->setSuffix( i18n( "min." ) );

    QHBoxLayout* lay = new QHBoxLayout;
    topLayout->addLayout(lay);
    lay->addWidget( label );
    lay->addWidget( m_autosave );
    lay->addStretch( 1 );

    // Backup
    lay = new QHBoxLayout;
    topLayout->addLayout(lay);
    QLabel* backupLabel = new QLabel( i18n( "Number of backups to keep:" ), this );
    lay->addWidget( backupLabel );

    m_backupCount = new QSpinBox;
    m_backupCount->setRange( -1, 100 );
    m_backupCount->setSpecialValueText( i18n( "Infinite" ) );
    lay->addWidget( m_backupCount );
    lay->addStretch( 1 );

    topLayout->addStretch(1);

    QString txt;
    txt = i18n("<p>KPhotoAlbum is capable of backing up the index.xml file by keeping copies named index.xml~1~ index.xml~2~ etc. "
               "and you can use the spinbox to specify the number of backup files to keep. "
               "KPhotoAlbum will delete the oldest backup file when it reaches "
               "the maximum number of backup files.</p>"
               "<p>The index.xml file may grow substantially if you have many images, and in that case it is useful to ask KPhotoAlbum to zip "
               "the backup files to preserve disk space.</p>" );
    backupLabel->setWhatsThis( txt );
    m_backupCount->setWhatsThis( txt );
    m_compressBackup->setWhatsThis( txt );

    txt = i18n( "<p>KPhotoAlbum is using a single index.xml file as its <i>data base</i>. With lots of images it may take "
                "a long time to read this file. You may cut down this time to approximately half, by checking this check box. "
                "The disadvantage is that the index.xml file is less readable by human eyes.</p>");
    m_compressedIndexXML->setWhatsThis( txt );
}

void Settings::DatabaseBackendPage::loadSettings( Settings::SettingsData* opt )
{
    m_compressedIndexXML->setChecked( opt->useCompressedIndexXML() );
    m_autosave->setValue( opt->autoSave() );
    m_backupCount->setValue( opt->backupCount() );
    m_compressBackup->setChecked( opt->compressBackup() );
}

void Settings::DatabaseBackendPage::saveSettings( Settings::SettingsData* opt )
{
    opt->setBackupCount( m_backupCount->value() );
    opt->setCompressBackup( m_compressBackup->isChecked() );
    opt->setUseCompressedIndexXML( m_compressedIndexXML->isChecked() );
    opt->setAutoSave( m_autosave->value() );
}

void Settings::DatabaseBackendPage::markDirty()
{
    MainWindow::DirtyIndicator::markDirty();
}
// vi:expandtab:tabstop=4 shiftwidth=4:
