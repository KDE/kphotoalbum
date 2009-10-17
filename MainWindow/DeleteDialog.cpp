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

#include "DB/ImageDB.h"
#include "DB/ImageInfo.h"
#include "DB/Result.h"
#include "DB/ResultId.h"
#include "ImageManager/Manager.h"
#include "Utilities/ShowBusyCursor.h"
#include "Utilities/Util.h"

using namespace MainWindow;

DeleteDialog::DeleteDialog( QWidget* parent )
    : KDialog(parent)
    , _list()
{
    setWindowTitle( i18n("Delete from database") );
    setButtons( Cancel|User1 );
    setButtonText( User1,i18n("Delete") );

    QWidget* top = new QWidget;
    QVBoxLayout* lay1 = new QVBoxLayout( top );
    setMainWidget( top );


    _label = new QLabel;
    lay1->addWidget( _label );

    _delete_file = new QCheckBox( i18n( "Delete file from disk as well" ) );
    lay1->addWidget( _delete_file );

    connect( this, SIGNAL( user1Clicked() ), this, SLOT( deleteImages() ) );
}

int DeleteDialog::exec(const DB::Result& list)
{
    _label->setText(
        i18n("<p><b><center><font size=\"+3\">"
             "Delete Images/Videos from database<br/>"
             "%1 selected"
             "</font></center></b></p>",
             list.size()));

    _delete_file->setChecked( true );
    _list = list;

    return KDialog::exec();
}

void DeleteDialog::deleteImages()
{
    Utilities::ShowBusyCursor dummy;

    DB::Result listToDelete;
    QStringList listCouldNotDelete;

    Q_FOREACH(const DB::ResultId id, _list) {
        const QString fileName = id.fetchInfo()->fileName(DB::AbsolutePath);
        if ( DB::ImageInfo::imageOnDisk( fileName ) ) {
            // TODO: should this probably call some KDE specific thing to
            // move the file in the Trash-bin or something ? Deleting
            // potentially precious images is a bit harsh.
            if ( _delete_file->isChecked() && !QFile( fileName ).remove() ) {
                listCouldNotDelete.append(fileName);
            } else {
                listToDelete.append(id);
                ImageManager::Manager::instance()->removeThumbnail( fileName );
            }
        } else {
            listToDelete.append(id);
        }
    }

    if( ! listCouldNotDelete.isEmpty()) {
        KMessageBox::errorList( this,
                                i18np("<p><b>Unable to physically delete a file. Do you have permission to do so?</b></p>",
                                      "<p><b>Unable to physically delete %1 files. Do you have permission to do so?</b></p>",
                                      listCouldNotDelete.count() ),
                                listCouldNotDelete,
                                i18np("Error Deleting File", "Error Deleting Files", listCouldNotDelete.count() ) );
    }

    if(!listToDelete.isEmpty()) {
        if ( _delete_file->isChecked() )
            DB::ImageDB::instance()->deleteList( listToDelete );
        else
            DB::ImageDB::instance()->addToBlockList( listToDelete );
        accept();
    } else {
        reject();
    }
}

#include "DeleteDialog.moc"
