/* SPDX-FileCopyrightText: 2012-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "DeleteFiles.h"

#include "ShowBusyCursor.h"

#include <DB/ImageDB.h>
#include <MainWindow/DirtyIndicator.h>
#include <MainWindow/Window.h>
#include <kpathumbnails/ThumbnailCache.h>

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

    DB::FileNameList filenamesToBlock; // will be removed and blocked
    DB::FileNameList filenamesToRemove; // will be removed but not blocked
    QList<QUrl> filesToDelete; // will be deleted/trashed

    for (const DB::FileName &fileName : files) {

        if (DB::ImageInfo::imageOnDisk(fileName)) {
            if (method == DeleteFromDisk || method == MoveToTrash) {
                filesToDelete.append(QUrl::fromLocalFile(fileName.absolute()));
                filenamesToRemove.append(fileName);
            } else {
                filenamesToBlock.append(fileName);
            }
        } else
            filenamesToRemove.append(fileName);
    }

    MainWindow::Window::theMainWindow()->thumbnailCache()->removeThumbnails(files);

    if (method == DeleteFromDisk || method == MoveToTrash) {
        KJob *job;
        if (method == MoveToTrash)
            job = KIO::trash(filesToDelete);
        else
            job = KIO::del(filesToDelete);
        connect(job, &KJob::result, this, &DeleteFiles::slotKIOJobCompleted);
    }

    if (!filenamesToRemove.isEmpty()) {
        DB::ImageDB::instance()->deleteList(filenamesToRemove);
    }
    if (!filenamesToBlock.isEmpty()) {
        DB::ImageDB::instance()->addToBlockList(filenamesToBlock);
    }
    return !(filenamesToBlock.isEmpty() && filenamesToRemove.isEmpty());
}

} // namespace Utilities
// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_DeleteFiles.cpp"
