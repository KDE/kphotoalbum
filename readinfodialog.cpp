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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "readinfodialog.h"
#include <klocale.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qbuttongroup.h>
#include <qfile.h>
#include <kmessagebox.h>
#include "imagedb.h"
#include "imageinfo.h"
#include "util.h"

ReadInfoDialog::ReadInfoDialog( QWidget* parent, const char* name )
    :KDialogBase( Plain, i18n("Read File Info"), Cancel|User1, User1, parent, name,
                  true, false, i18n("Read File Info"))
{
    QWidget* top = plainPage();
    QVBoxLayout* lay1 = new QVBoxLayout( top, 6 );

    _label = new QLabel( top );
    lay1->addWidget( _label );

    _time = new QCheckBox( i18n( "Read time" ), top );
    lay1->addWidget( _time );

    _force_time = new QCheckBox( i18n( "Use modification time if EXIF not present" ), top );
    lay1->addWidget( _force_time );

    _date = new QCheckBox( i18n( "Read date" ), top );
    lay1->addWidget( _date );

    _force_date = new QCheckBox( i18n( "Use modification date if EXIF not present" ), top );
    lay1->addWidget( _force_date );

    _orientation = new QCheckBox( i18n( "Read EXIF orientation" ), top );
    lay1->addWidget( _orientation );

    _description = new QCheckBox( i18n( "Read EXIF description" ), top );
    lay1->addWidget( _description );

    connect( this, SIGNAL( user1Clicked() ), this, SLOT( readInfo() ) );
}

int ReadInfoDialog::exec( const ImageInfoList& list )
{
    _label->setText( i18n("<qt><b><center><font size=\"+3\">Read File Info<br>%1 selected</font></center></b></qt>").arg( list.count() ) );

    _time->setChecked( true );
    _force_time->setChecked( true );
    _date->setChecked( true );
    _force_date->setChecked( true );
    _orientation->setChecked( false );
    _description->setChecked( false );
    _list = list;

    return KDialogBase::exec();
}

void ReadInfoDialog::readInfo()
{
    int mode = EXIFMODE_FORCE;

    if ( _time->isChecked() )
            mode |= EXIFMODE_TIME;
    if ( _force_time->isChecked() )
            mode |= EXIFMODE_FORCE_TIME;
    if ( _date->isChecked() )
            mode |= EXIFMODE_DATE;
    if ( _force_date->isChecked() )
            mode |= EXIFMODE_FORCE_DATE;
    if ( _orientation->isChecked() )
            mode |= EXIFMODE_ORIENTATION;
    if ( _description->isChecked() )
            mode |= EXIFMODE_DESCRIPTION;

    ImageDB::instance()->slotReread(_list, mode);
    accept();
}

#include "readinfodialog.moc"
