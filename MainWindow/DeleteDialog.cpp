/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
#include <kmessagebox.h>
#include <qcheckbox.h>
#include <qfile.h>
#include <qlabel.h>
#include <qlayout.h>
#include <kio/deletejob.h>
#include "kio/copyjob.h"
#include "DB/ImageDB.h"
#include "DB/ImageInfo.h"
#include "DB/Result.h"
#include "DB/ResultId.h"
#include "ImageManager/Manager.h"
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

    _use_trash = new QRadioButton( i18n("Move to Trash") );
    lay1->addWidget( _use_trash );

    _delete_file = new QRadioButton( i18n( "Delete from disk" ) );
    lay1->addWidget( _delete_file );

    _delete_from_db = new QRadioButton( i18n( "Leave to disk" ) );
    lay1->addWidget( _delete_from_db );

     connect( this, SIGNAL( user1Clicked() ), this, SLOT( deleteImages() ) );
}

int DeleteDialog::exec(const DB::Result& list)
{
    if (!list.size()) return 0;

    _label->setText(
        i18n("<p><b><center><font size=\"+3\">"
             "Removing %1 %2<br />"
             "</font>"
             "Selected %2 %4 removed from database. <br />"
             "What do you want to do with the %3 on disk?"
             "</center></b></p>",
             list.size(), list.size() == 1 ? i18n("item") : i18n("items"),
             list.size() == 1 ? i18n("file") : i18n("files"),
             list.size() == 1 ? i18n("is") : i18n("are")
        )
    );

    _use_trash->setChecked( true );
    _list = list;

    return KDialog::exec();
}

void DeleteDialog::deleteImages()
{
    Utilities::ShowBusyCursor dummy;
    DB::Result listToDelete;
    KUrl::List listKUrlToDelete;
    KUrl KUrlToDelete;

    Q_FOREACH(const DB::ResultId id, _list) {
        const QString fileName = id.fetchInfo()->fileName(DB::AbsolutePath);
        if ( DB::ImageInfo::imageOnDisk( fileName ) ) {
            if ( _delete_file->isChecked() || _use_trash->isChecked() ){
                KUrlToDelete.setPath(fileName);
                listKUrlToDelete.append(KUrlToDelete);
                listToDelete.append(id);
                ImageManager::Manager::instance()->removeThumbnail( fileName );
            } else {
                listToDelete.append(id);
                ImageManager::Manager::instance()->removeThumbnail( fileName );
            }
        } else
            listToDelete.append(id);
    }

    if ( _delete_file->isChecked() || _use_trash->isChecked() ) {
        KJob* job;
        if ( _use_trash->isChecked() )
            job = KIO::trash( listKUrlToDelete );
        else
            job = KIO::del( listKUrlToDelete );
        connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotKIOJobCompleted( KJob* ) ) );
    }

    if(!listToDelete.isEmpty()) {
        if ( _delete_file->isChecked() || _use_trash->isChecked() )
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
