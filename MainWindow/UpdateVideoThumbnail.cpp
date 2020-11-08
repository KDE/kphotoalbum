/* SPDX-FileCopyrightText: 2012-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "UpdateVideoThumbnail.h"

#include "Logging.h"
#include "Window.h"

#include <BackgroundJobs/HandleVideoThumbnailRequestJob.h>
#include <ImageManager/ThumbnailCache.h>
#include <ThumbnailView/CellGeometry.h>
#include <Utilities/FileUtil.h>
#include <Utilities/VideoUtil.h>

#include <QLoggingCategory>

namespace
{

DB::FileName nextExistingImage(const DB::FileName &fileName, int frame, int direction)
{
    // starting with i=1 is *not* an off-by-one error
    for (int i = 1; i < 10; ++i) {
        const int nextIndex = (frame + 10 + direction * i) % 10;
        const DB::FileName file = BackgroundJobs::HandleVideoThumbnailRequestJob::frameName(fileName, nextIndex);
        if (file.exists()) {
            qCDebug(MainWindowLog) << "Next frame for video" << file.relative() << " has index" << nextIndex;
            return file;
        }
    }
    const DB::FileName file = BackgroundJobs::HandleVideoThumbnailRequestJob::frameName(fileName, 0);
    qCDebug(MainWindowLog) << "No next thumbnail found for video" << fileName.relative()
                           << "(frame:" << frame << ", direction:" << direction << ") - thumbnail base name:" << file.absolute();
    // the thumbnail may not have been written to disk yet
    return DB::FileName();
}

void update(const DB::FileName &fileName, int direction)
{
    const DB::FileName baseImageName = BackgroundJobs::HandleVideoThumbnailRequestJob::pathForRequest(fileName);
    QImage baseImage(baseImageName.absolute());

    int frame = 0;
    for (; frame < 10; ++frame) {
        const DB::FileName frameFile = BackgroundJobs::HandleVideoThumbnailRequestJob::frameName(fileName, frame);
        QImage frameImage(frameFile.absolute());
        if (frameImage.isNull())
            continue;
        if (baseImage == frameImage) {
            break;
        }
    }

    const DB::FileName newImageName = nextExistingImage(fileName, frame, direction);
    if (newImageName.isNull())
        return;

    Utilities::copyOrOverwrite(newImageName.absolute(), baseImageName.absolute());

    QImage image = QImage(newImageName.absolute()).scaled(ThumbnailView::CellGeometry::preferredIconSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    MainWindow::Window::theMainWindow()->thumbnailCache()->insert(fileName, image);
    MainWindow::Window::theMainWindow()->reloadThumbnails();
}

void update(const DB::FileNameList &list, int direction)
{
    for (const DB::FileName &fileName : list) {
        if (Utilities::isVideo(fileName))
            update(fileName, direction);
    }
}
} // namespace

void MainWindow::UpdateVideoThumbnail::useNext(const DB::FileNameList &list)
{
    update(list, +1);
}

void MainWindow::UpdateVideoThumbnail::usePrevious(const DB::FileNameList &list)
{
    update(list, -1);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
