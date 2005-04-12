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

#include "deletedialog.h"
#include <klocale.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qbuttongroup.h>
#include <qfile.h>
#include <kmessagebox.h>
#include "imagedb.h"
#include "util.h"
#include "imageinfo.h"

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

int DeleteDialog::exec( const ImageInfoList& list )
{
    _label->setText( i18n("<qt><b><center><font size=\"+3\">Delete Images<br>%1 selected</font></center></b></qt>").arg( list.count() ) );

    bool onDisk = false;
    for( ImageInfoListIterator it( list ); *it; ++it ) {
        onDisk |= (*it)->imageOnDisk();
    }

    _deleteFromDisk->setChecked( true );
    _block->setChecked( false );
    _list = list;

    return KDialogBase::exec();
}

void DeleteDialog::deleteImages()
{
    if ( _deleteFromDisk->isChecked() ) {
        for( ImageInfoListIterator it( _list ); *it; ++it ) {
            Util::removeThumbNail( (*it)->fileName() );
            if ( (*it)->imageOnDisk() ) {
                bool ok = QFile( (*it)->fileName() ).remove();
                if ( !ok ) {
                    KMessageBox::error( this, i18n("Unable to delete file '%1'.").arg((*it)->fileName()),
                                        i18n("Error Deleting Files") );
                }
            }
        }
    }

    if ( _block->isChecked() )
        ImageDB::instance()->blockList( _list );

    if ( _deleteFromDisk->isChecked() )
        ImageDB::instance()->deleteList( _list );

    accept();
}

#include "deletedialog.moc"
