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
#include "ImageManager/ThumbnailCache.h"

#include <QVBoxLayout>
#include <klocale.h>
#include <kmessagebox.h>
#include <qcheckbox.h>
#include <qfile.h>
#include <qlabel.h>
#include <qlayout.h>
#include <kio/deletejob.h>
#include "kio/copyjob.h"
#include "DB/ImageDB.h"
#include "DB/ImageInfo.h"
#include "Utilities/ShowBusyCursor.h"
#include "Utilities/Util.h"
#include "MainWindow/DirtyIndicator.h"

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

     connect( this, SIGNAL( user1Clicked() ), this, SLOT( deleteImages() ) );
}

int DeleteDialog::exec(const DB::FileNameList& list)
{
    if (!list.size()) return 0;

    const QString msg1 = i18np( "Removing 1 item", "Removing %1 items", list.size() );
    const QString msg2 = i18np( "Selected item will be removed from the database.<br/>What do you want to do with the file on disk?",
                                "Selected %1 items will be removed from the database.<br/>What do you want to do with the files on disk?",
                                list.size() );

    const QString txt = QString::fromLatin1( "<p><b><center><font size=\"+3\">%1</font><br/>%2</center></b></p>" ).arg(msg1).arg(msg2);

    _useTrash->setText( i18np("Move file to Trash", "Move %1 files to Trash", list.size() ) );
    _deleteFile->setText( i18np( "Delete file from disk", "Delete %1 files from disk", list.size() ) );
    _deleteFromDb->setText( i18np( "Only remove the item from database", "Only remove %1 items from database", list.size() ) );


    _label->setText( txt );
    _useTrash->setChecked( true );
    _list = list;

    return KDialog::exec();
}

void DeleteDialog::deleteImages()
{
    Utilities::ShowBusyCursor dummy;
    DB::FileNameList listToDelete;
    KUrl::List listKUrlToDelete;
    KUrl KUrlToDelete;

    Q_FOREACH(const DB::FileName fileName, _list) {
        if ( DB::ImageInfo::imageOnDisk( fileName ) ) {
            if ( _deleteFile->isChecked() || _useTrash->isChecked() ){
                KUrlToDelete.setPath(fileName.absolute());
                listKUrlToDelete.append(KUrlToDelete);
                listToDelete.append(fileName);
                ImageManager::ThumbnailCache::instance()->removeThumbnail( fileName );
            } else {
                listToDelete.append(fileName);
                ImageManager::ThumbnailCache::instance()->removeThumbnail( fileName );
            }
        } else
            listToDelete.append(fileName);
    }

    if ( _deleteFile->isChecked() || _useTrash->isChecked() ) {
        KJob* job;
        if ( _useTrash->isChecked() )
            job = KIO::trash( listKUrlToDelete );
        else
            job = KIO::del( listKUrlToDelete );
        connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotKIOJobCompleted( KJob* ) ) );
    }

    if(!listToDelete.isEmpty()) {
        if ( _deleteFile->isChecked() || _useTrash->isChecked() )
            DB::ImageDB::instance()->deleteList( listToDelete );
        else
            DB::ImageDB::instance()->addToBlockList( listToDelete );
        MainWindow::DirtyIndicator::markDirty();
        accept();
    } else {
        reject();
    }
}

void DeleteDialog::slotKIOJobCompleted( KJob* job)
{
    if ( job->error() )
        KMessageBox::error( this, job->errorString(), i18n( "Error Deleting Files" ) );
}

#include "DeleteDialog.moc"
