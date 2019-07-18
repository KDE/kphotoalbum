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

#include "ShowBusyCursor.h"

#include <DB/ImageDB.h>
#include <ImageManager/ThumbnailCache.h>
#include <MainWindow/DirtyIndicator.h>
#include <MainWindow/Window.h>

#include <KIO/CopyJob>
#include <KIO/DeleteJob>
#include <KJob>
#include <KLocalizedString>
#include <KMessageBox>

#include <QUrl>

namespace Utilities
{

DeleteFiles *DeleteFiles::s_instance;

bool DeleteFiles::deleteFiles(const DB::FileNameList &files, DeleteMethod method)
{
    if (!s_instance)
        s_instance = new DeleteFiles;
    return s_instance->deleteFilesPrivate(files, method);
}

void DeleteFiles::slotKIOJobCompleted(KJob *job)
{
    if (job->error())
        KMessageBox::error(MainWindow::Window::theMainWindow(), job->errorString(), i18n("Error Deleting Files"));
}

bool DeleteFiles::deleteFilesPrivate(const DB::FileNameList &files, DeleteMethod method)
{
    Utilities::ShowBusyCursor dummy;

    DB::FileNameList filenamesToRemove;
    QList<QUrl> filesToDelete;

    Q_FOREACH (const DB::FileName &fileName, files) {

        if (DB::ImageInfo::imageOnDisk(fileName)) {
            if (method == DeleteFromDisk || method == MoveToTrash) {
                filesToDelete.append(QUrl::fromLocalFile(fileName.absolute()));
                filenamesToRemove.append(fileName);
            } else {
                filenamesToRemove.append(fileName);
            }
        } else
            filenamesToRemove.append(fileName);
    }

    ImageManager::ThumbnailCache::instance()->removeThumbnails(files);

    if (method == DeleteFromDisk || method == MoveToTrash) {
        KJob *job;
        if (method == MoveToTrash)
            job = KIO::trash(filesToDelete);
        else
            job = KIO::del(filesToDelete);
        connect(job, SIGNAL(result(KJob *)), this, SLOT(slotKIOJobCompleted(KJob *)));
    }

    if (!filenamesToRemove.isEmpty()) {
        if (method == MoveToTrash || method == DeleteFromDisk)
            DB::ImageDB::instance()->deleteList(filenamesToRemove);
        else
            DB::ImageDB::instance()->addToBlockList(filenamesToRemove);
        MainWindow::DirtyIndicator::markDirty();
        return true;
    } else
        return false;
}

} // namespace Utilities
// vi:expandtab:tabstop=4 shiftwidth=4:
