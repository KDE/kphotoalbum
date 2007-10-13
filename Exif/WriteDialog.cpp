/* Copyright (C) 2003-2007 Jesper K. Pedersen <blackie@kde.org>, Jan Kundrát <jkt@gentoo.org>

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

#include "Exif/WriteDialog.h"
#include <klocale.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <kmessagebox.h>
#include "DB/ImageDB.h"
#include "Exif/Database.h"


Exif::WriteDialog::WriteDialog( QWidget* parent, const char* name )
    :KDialogBase( Plain, i18n("Write File Metadata"), Cancel|User1|User2, User1, parent, name,
                  true, false, i18n("Write"), i18n("Show File List") )
{
    QWidget* top = plainPage();
    QVBoxLayout* lay1 = new QVBoxLayout( top, 6 );

    _title = new QLabel( top );
    lay1->addWidget( _title );

    _label = new QCheckBox( i18n( "Update label" ), top );
    lay1->addWidget( _label );

    _description = new QCheckBox( i18n( "Update description" ), top );
    lay1->addWidget( _description );

    _orientation = new QCheckBox( i18n( "Update image orientation" ), top );
    lay1->addWidget( _orientation );

    _date = new QCheckBox( i18n( "Update date and time" ), top );
    lay1->addWidget( _date );

    _categories = new QCheckBox( i18n( "Export tags" ), top );
    lay1->addWidget( _categories );

    connect( this, SIGNAL( user1Clicked() ), this, SLOT( write() ) );
    connect( this, SIGNAL( user2Clicked() ), this, SLOT( showFileList() ) );
}

int Exif::WriteDialog::exec( const QStringList& list )
{
    QString titleCaption = i18n("<p><b><center><font size=\"+3\">Write File Info</font><br>%1 selected</center></b></p>").arg( list.count() );
    _title->setText( titleCaption );

    _label->setChecked( true );
    _description->setChecked( true );
    _orientation->setChecked( true );
    _date->setChecked( true );
    _categories->setChecked( true );

    _list = list;

    return KDialogBase::exec();
}

void Exif::WriteDialog::write()
{
    int mode = 0;

    if ( _date->isChecked() )
            mode |= EXIFMODE_DATE;
    if ( _orientation->isChecked() )
            mode |= EXIFMODE_ORIENTATION;
    if ( _description->isChecked() )
            mode |= EXIFMODE_DESCRIPTION;
    if ( _label->isChecked() )
            mode |= EXIFMODE_LABEL;
    if ( _categories->isChecked() )
            mode |= EXIFMODE_CATEGORIES;

    if ( ( mode & ~EXIFMODE_DATABASE_UPDATE ) && !warnAboutChanges() )
        return;

    accept();
    DB::ImageDB::instance()->slotWrite(_list, mode);
}

void Exif::WriteDialog::showFileList()
{
    int i = KMessageBox::warningContinueCancelList( this,
                                                    i18n( "<p><b>%1 files</b> are affected by this operation, their filenames "
                                                          "can be seen in the list below.</p>").arg(_list.count()), _list,
                                                    i18n("Files affected"),
                                                    KStdGuiItem::cont(),
                                                    QString::fromLatin1( "writeEXIFinfoIsDangerous" ) );
    if ( i == KMessageBox::Cancel )
        return;
}

/*
 * Warn user that this might nuke her data. Return true if she wants to proceed
 * anyway.
 */
bool Exif::WriteDialog::warnAboutChanges()
{
    int ret = KMessageBox::warningYesNo( this, i18n("<p>Be aware that this action might be <b>dangerous</b> "
                                                    "as it overwrites your image files. Use at your own risk.</p>"),
                                         i18n( "Override image dates" ) );
    return ret == KMessageBox::Yes;
}

#include "WriteDialog.moc"
