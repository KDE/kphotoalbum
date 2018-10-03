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

#include "ReReadDialog.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>

#include <DB/ImageDB.h>
#include <Exif/Database.h>
#include <Settings/SettingsData.h>


Exif::ReReadDialog::ReReadDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle( i18nc("@title:window", "Read Exif Info from Files") );

    QWidget* top = new QWidget;
    QVBoxLayout* lay1 = new QVBoxLayout( top );
    setLayout(lay1);
    lay1->addWidget(top);

    m_exifDB = new QCheckBox( i18n( "Update Exif search database" ), top );
    lay1->addWidget( m_exifDB );
    if ( !Exif::Database::instance()->isUsable() ) {
        m_exifDB->hide();
    }

    m_date = new QCheckBox( i18n( "Update image date" ), top );
    lay1->addWidget( m_date );

    m_force_date = new QCheckBox( i18n( "Use modification date if Exif not found" ), top );
    lay1->addWidget( m_force_date );

    m_orientation = new QCheckBox( i18n( "Update image orientation from Exif information" ), top );
    lay1->addWidget( m_orientation );

    m_description = new QCheckBox( i18n( "Update image description from Exif information" ), top );
    lay1->addWidget( m_description );

    QGroupBox* box = new QGroupBox( i18n("Affected Files") );
    lay1->addWidget( box );

    QHBoxLayout* boxLayout = new QHBoxLayout( box );
    m_fileList = new QListWidget;
    m_fileList->setSelectionMode( QAbstractItemView::NoSelection );
    boxLayout->addWidget( m_fileList );


    connect( m_date, SIGNAL(toggled(bool)), m_force_date, SLOT(setEnabled(bool)) );
    connect( m_date, SIGNAL(toggled(bool)), this, SLOT(warnAboutDates(bool)) );

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Ok)->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ReReadDialog::readInfo);
    lay1->addWidget(buttonBox);
}

int Exif::ReReadDialog::exec( const DB::FileNameList& list )
{
    Settings::SettingsData *opt = Settings::SettingsData::instance();

    m_exifDB->setChecked( opt->updateExifData() );
    m_date->setChecked( opt->updateImageDate() );
    m_force_date->setChecked( opt->useModDateIfNoExif() );
    m_force_date->setEnabled( opt->updateImageDate() );
    m_orientation->setChecked( opt->updateOrientation() );
    m_description->setChecked( opt->updateDescription() );

    m_list = list;
    m_fileList->clear();
    m_fileList->addItems( list.toStringList(DB::RelativeToImageRoot) );

    return QDialog::exec();
}

void Exif::ReReadDialog::readInfo()
{
    Settings::SettingsData *opt = Settings::SettingsData::instance();

    opt->setUpdateExifData( m_exifDB->isChecked() );
    opt->setUpdateImageDate( m_date->isChecked() );
    opt->setUseModDateIfNoExif( m_force_date->isChecked() );
    opt->setUpdateOrientation( m_orientation->isChecked() );
    opt->setUpdateDescription( m_description->isChecked() );

    KSharedConfig::openConfig()->sync();

    DB::ExifMode mode = DB::EXIFMODE_FORCE;

    if ( m_exifDB->isChecked() )
        mode |= DB::EXIFMODE_DATABASE_UPDATE;

    if ( m_date->isChecked() )
            mode |= DB::EXIFMODE_DATE;
    if ( m_force_date->isChecked() )
             mode |= DB::EXIFMODE_USE_IMAGE_DATE_IF_INVALID_EXIF_DATE;
    if ( m_orientation->isChecked() )
            mode |= DB::EXIFMODE_ORIENTATION;
    if ( m_description->isChecked() )
            mode |= DB::EXIFMODE_DESCRIPTION;

    accept();
    DB::ImageDB::instance()->slotReread(m_list, mode);
}

void Exif::ReReadDialog::warnAboutDates( bool b )
{
    if ( !b )
        return;

    int ret = KMessageBox::warningContinueCancel( this, i18n("<p>Be aware that setting the data from Exif may "
                                                    "<b>overwrite</b> data you have previously entered "
                                                    "manually using the image configuration dialog.</p>" ),
                                         i18n( "Override image dates" ) );
    if ( ret == KMessageBox::Cancel )
        m_date->setChecked( false );
}

// vi:expandtab:tabstop=4 shiftwidth=4:
