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

#include "DeleteDialog.h"
#include <klocale.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qfile.h>
#include <kmessagebox.h>
#include "DB/ImageDB.h"
#include "Utilities/Util.h"
#include "DB/ImageInfo.h"
#include "Utilities/ShowBusyCursor.h"

using namespace MainWindow;

DeleteDialog::DeleteDialog( QWidget* parent, const char* name )
    :KDialogBase( Plain, i18n("Delete Images"), Cancel|User1, User1, parent, name,
                  true, false, KGuiItem(i18n("Delete Images"),QString::fromLatin1("editdelete")))
{
    QWidget* top = plainPage();
    QVBoxLayout* lay1 = new QVBoxLayout( top, 6 );

    _label = new QLabel( top );
    lay1->addWidget( _label );

    _deleteFromDisk = new QCheckBox( i18n( "Delete images from disk and database" ), top );
    lay1->addWidget( _deleteFromDisk );

    _block = new QCheckBox( i18n( "Block from database" ), top );
    lay1->addWidget( _block );

    connect( this, SIGNAL( user1Clicked() ), this, SLOT( deleteImages() ) );
}

int DeleteDialog::exec( const QStringList& list )
{
    _label->setText( i18n("<qt><b><center><font size=\"+3\">Delete Images<br>%1 selected</font></center></b></qt>").arg( list.count() ) );

    _deleteFromDisk->setChecked( true );
    _block->setChecked( false );
    _list = list;

    return KDialogBase::exec();
}

void DeleteDialog::deleteImages()
{
    Utilities::ShowBusyCursor dummy;

    if ( _deleteFromDisk->isChecked() ) {
        for( QStringList::ConstIterator it = _list.begin(); it != _list.end(); ++it ) {
            Utilities::removeThumbNail( *it );
            if ( DB::ImageInfo::imageOnDisk(*it) ) {
                bool ok = !(QFile( *it ).exists()) ||  QFile( *it ).remove();
                if ( !ok ) {
                    KMessageBox::error( this, i18n("Unable to delete file '%1'.").arg(*it),
                                        i18n("Error Deleting Files") );
                }
            }
        }
    }

    if ( _block->isChecked() )
        DB::ImageDB::instance()->addToBlockList( _list );

    if ( _deleteFromDisk->isChecked() )
        DB::ImageDB::instance()->deleteList( _list );

    accept();
}

#include "DeleteDialog.moc"
