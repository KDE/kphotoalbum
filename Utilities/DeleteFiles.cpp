/* Copyright 2012 Jesper K. Pedersen <blackie@kde.org>
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "DeleteFiles.h"
#include "MainWindow/Window.h"
#include <KJob>
#include <KLocale>
#include <KMessageBox>
#include "ShowBusyCursor.h"
#include "ImageManager/ThumbnailCache.h"
#include <kio/deletejob.h>
#include "kio/copyjob.h"
#include "DB/ImageDB.h"
#include "MainWindow/DirtyIndicator.h"

namespace Utilities {

DeleteFiles* DeleteFiles::s_instance;

bool DeleteFiles::deleteFiles( const DB::FileNameList& files, DeleteMethod method )
{
    if (!s_instance)
        s_instance = new DeleteFiles;
    return s_instance->_deleteFiles(files,method);
}

void DeleteFiles::slotKIOJobCompleted(KJob* job)
{
    if ( job->error() )
        KMessageBox::error( MainWindow::Window::theMainWindow(), job->errorString(), i18n( "Error Deleting Files" ) );
}

bool DeleteFiles::_deleteFiles(const DB::FileNameList &files, DeleteMethod method)
{
    Utilities::ShowBusyCursor dummy;

    DB::FileNameList listToDelete;
    KUrl::List listKUrlToDelete;
    KUrl KUrlToDelete;

    Q_FOREACH(const DB::FileName fileName, files) {
        ImageManager::ThumbnailCache::instance()->removeThumbnail( fileName );

        if ( DB::ImageInfo::imageOnDisk( fileName ) ) {
            if ( method == DeleteFromDisk || method == MoveToTrash ){
                KUrlToDelete.setPath(fileName.absolute());
                listKUrlToDelete.append(KUrlToDelete);
                listToDelete.append(fileName);
            } else {
                listToDelete.append(fileName);
            }
        }
        else
            listToDelete.append(fileName);
    }

    if ( method == DeleteFromDisk || method == MoveToTrash ) {
        KJob* job;
        if ( method == MoveToTrash )
            job = KIO::trash( listKUrlToDelete );
        else
            job = KIO::del( listKUrlToDelete );
        connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotKIOJobCompleted( KJob* ) ) );
    }

    if(!listToDelete.isEmpty()) {
        if ( method == MoveToTrash || method == DeleteFromDisk )
            DB::ImageDB::instance()->deleteList( listToDelete );
        else
            DB::ImageDB::instance()->addToBlockList( listToDelete );
        MainWindow::DirtyIndicator::markDirty();
        return true;
    }
    else
        return false;
}


} // namespace Utilities
// vi:expandtab:tabstop=4 shiftwidth=4:
