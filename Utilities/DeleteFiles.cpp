/* SPDX-FileCopyrightText: 2012-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

    for (const DB::FileName &fileName : files) {

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
