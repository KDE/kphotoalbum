// SPDX-FileCopyrightText: 2012 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2013 - 2024 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2020 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "UpdateVideoThumbnail.h"

#include "Logging.h"
#include "Window.h"
#include "kpathumbnails/VideoThumbnailCache.h"

#include <BackgroundJobs/HandleVideoThumbnailRequestJob.h>
#include <ThumbnailView/CellGeometry.h>
#include <kpabase/FileExtensions.h>
#include <kpathumbnails/ThumbnailCache.h>

#include <QLoggingCategory>

namespace
{

void update(const DB::FileName &fileName, int direction)
{
    auto videoThumbnailCache = MainWindow::Window::theMainWindow()->videoThumbnailCache();
    int frameNumber = videoThumbnailCache->stillFrameIndex(fileName);

    int frame = frameNumber + direction;
    while (0 <= frame && frame < videoThumbnailCache->numberOfFrames()) {
        if (videoThumbnailCache->setStillFrame(fileName, frame))
            break;
        frame += direction;
    }
    qCDebug(MainWindowLog) << "No next thumbnail found for video" << fileName.relative()
                           << "(start frame:" << frameNumber << ", direction:" << direction << ")";

    QImage image = videoThumbnailCache->lookupStillFrame(fileName).scaled(ThumbnailView::CellGeometry::preferredIconSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    MainWindow::Window::theMainWindow()->thumbnailCache()->insert(fileName, image);
    MainWindow::Window::theMainWindow()->reloadThumbnails();
}

void update(const DB::FileNameList &list, int direction)
{
    for (const DB::FileName &fileName : list) {
        if (KPABase::isVideo(fileName))
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
