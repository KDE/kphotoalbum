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

#include "DeleteDialog.h"

#include <QVBoxLayout>
#include <klocale.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include "Utilities/DeleteFiles.h"

using namespace MainWindow;

DeleteDialog::DeleteDialog( QWidget* parent )
    : KDialog(parent)
    , _list()
{
    setWindowTitle( i18n("Removing items") );
    setButtons( Cancel|User1 );
    setButtonText( User1,i18n("OK") );

    QWidget* top = new QWidget;
    QVBoxLayout* lay1 = new QVBoxLayout( top );
    setMainWidget( top );


    _label = new QLabel;
    lay1->addWidget( _label );

    _useTrash = new QRadioButton;
    lay1->addWidget( _useTrash );

    _deleteFile = new QRadioButton;
    lay1->addWidget( _deleteFile );

    _deleteFromDb = new QRadioButton;
    lay1->addWidget( _deleteFromDb );

     connect( this, SIGNAL(user1Clicked()), this, SLOT(deleteImages()) );
}

int DeleteDialog::exec(const DB::FileNameList& list)
{
    if (!list.size()) return 0;

    bool someFileExists = false;
    Q_FOREACH(const DB::FileName& file, list) {
        if ( file.exists() ) {
            someFileExists = true;
            break;
        }
    }

    const QString msg1 = i18np( "Removing 1 item", "Removing %1 items", list.size() );
    const QString msg2 = i18np( "Selected item will be removed from the database.<br/>What do you want to do with the file on disk?",
                                "Selected %1 items will be removed from the database.<br/>What do you want to do with the files on disk?",
                                list.size() );

    const QString txt = QString::fromLatin1( "<p><b><center><font size=\"+3\">%1</font><br/>%2</center></b></p>" ).arg(msg1).arg(msg2);

    _useTrash->setText( i18np("Move file to Trash", "Move %1 files to Trash", list.size() ) );
    _deleteFile->setText( i18np( "Delete file from disk", "Delete %1 files from disk", list.size() ) );
    _deleteFromDb->setText( i18np( "Only remove the item from database", "Only remove %1 items from database", list.size() ) );

    _label->setText( txt );
    _list = list;

    // disable trash/delete options if files don't exist
    _useTrash->setChecked( someFileExists );
    _useTrash->setEnabled( someFileExists );
    _deleteFile->setEnabled( someFileExists );
    _deleteFromDb->setChecked( !someFileExists );

    return KDialog::exec();
}

void DeleteDialog::deleteImages()
{
    bool anyDeleted = Utilities::DeleteFiles::deleteFiles(_list, _deleteFile->isChecked() ? Utilities::DeleteFromDisk :
                                                             _useTrash->isChecked() ? Utilities::MoveToTrash : Utilities::BlockFromDatabase );
    if ( anyDeleted )
        accept();
    else
        reject();
}

#include "DeleteDialog.moc"
// vi:expandtab:tabstop=4 shiftwidth=4:
