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

#include "Exif/ReReadDialog.h"
#include <klocale.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <kmessagebox.h>
#include "DB/ImageDB.h"
#include "Exif/Database.h"


Exif::ReReadDialog::ReReadDialog( QWidget* parent, const char* name )
    :KDialogBase( Plain, i18n("Read File Info"), Cancel|User1|User2, User1, parent, name,
                  true, false, i18n("Read File Info"), i18n("Show File List") )
{
    QWidget* top = plainPage();
    QVBoxLayout* lay1 = new QVBoxLayout( top, 6 );

    _label = new QLabel( top );
    lay1->addWidget( _label );

    _exifDB = new QCheckBox( i18n( "Update EXIF search database" ), top );
    lay1->addWidget( _exifDB );
    if ( !Exif::Database::instance()->isUsable() ) {
        _exifDB->hide();
    }

    _date = new QCheckBox( i18n( "Update image date" ), top );
    lay1->addWidget( _date );

    _force_date = new QCheckBox( i18n( "Use modification date if EXIF not found" ), top );
    lay1->addWidget( _force_date );

    _orientation = new QCheckBox( i18n( "Update image orientation from EXIF information" ), top );
    lay1->addWidget( _orientation );

    _description = new QCheckBox( i18n( "Update image description from EXIF information" ), top );
    lay1->addWidget( _description );

    connect( this, SIGNAL( user1Clicked() ), this, SLOT( readInfo() ) );
    connect( this, SIGNAL( user2Clicked() ), this, SLOT( showFileList() ) );
    connect( _date, SIGNAL( toggled( bool ) ), _force_date, SLOT( setEnabled( bool ) ) );
    connect( _date, SIGNAL( toggled( bool ) ), this, SLOT( warnAboutDates( bool ) ) );
}

int Exif::ReReadDialog::exec( const QStringList& list )
{
    _label->setText( i18n("<qt><b><center><font size=\"+3\">Read File Info<br>%1 selected</font></center></b></qt>").arg( list.count() ) );

    _exifDB->setChecked( true);
    _date->setChecked( false );
    _force_date->setChecked( true );
    _force_date->setEnabled( false );
    _orientation->setChecked( false );
    _description->setChecked( false );
    _list = list;

    return KDialogBase::exec();
}

void Exif::ReReadDialog::readInfo()
{
    int mode = EXIFMODE_FORCE;

    if ( _exifDB->isChecked() )
        mode |= EXIFMODE_DATABASE_UPDATE;

    if ( _date->isChecked() )
            mode |= EXIFMODE_DATE;
    if ( _force_date->isChecked() )
            mode |= EXIFMODE_FORCE_DATE;
    if ( _orientation->isChecked() )
            mode |= EXIFMODE_ORIENTATION;
    if ( _description->isChecked() )
            mode |= EXIFMODE_DESCRIPTION;

    accept();
    DB::ImageDB::instance()->slotReread(_list, mode);
}

void Exif::ReReadDialog::showFileList()
{
    int i = KMessageBox::warningContinueCancelList( this,
                                                    i18n( "<qt><b>%1 files</b> are affected by this operation, their filenames "
                                                          "can be seen in the list below.</p></qt>").arg(_list.count()), _list,
                                                    i18n("Files affected"),
                                                    KStdGuiItem::cont(),
                                                    QString::fromLatin1( "readEXIFinfoIsDangerous" ) );
    if ( i == KMessageBox::Cancel )
        return;
}

void Exif::ReReadDialog::warnAboutDates( bool b )
{
    if ( !b )
        return;

    int ret = KMessageBox::warningYesNo( this, i18n("<qt><p>Be aware that setting the data from EXIF may "
                                                    "<b>overwrite</b> data you have previously entered "
                                                    "manually using the image configuration dialog.</p></qt>" ),
                                         i18n( "Override image dates" ) );
    if ( ret == KMessageBox::No )
        _date->setChecked( false );
}

#include "ReReadDialog.moc"
