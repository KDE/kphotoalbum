/* Copyright (C) 2012-2020 The KPhotoAlbum Development Team

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e. V. (or its successor approved
   by the membership of KDE e. V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "UpdateVideoThumbnail.h"

#include "Window.h"

#include <BackgroundJobs/HandleVideoThumbnailRequestJob.h>
#include <ImageManager/ThumbnailCache.h>
#include <ThumbnailView/CellGeometry.h>
#include <Utilities/FileUtil.h>
#include <Utilities/VideoUtil.h>

namespace MainWindow
{

void UpdateVideoThumbnail::useNext(const DB::FileNameList &list)
{
    update(list, +1);
}

void UpdateVideoThumbnail::usePrevious(const DB::FileNameList &list)
{
    update(list, -1);
}

void UpdateVideoThumbnail::update(const DB::FileNameList &list, int direction)
{
    for (const DB::FileName &fileName : list) {
        if (Utilities::isVideo(fileName))
            update(fileName, direction);
    }
}

void UpdateVideoThumbnail::update(const DB::FileName &fileName, int direction)
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

    Utilities::copyOrOverwrite(newImageName.absolute(), baseImageName.absolute());

    QImage image = QImage(newImageName.absolute()).scaled(ThumbnailView::CellGeometry::preferredIconSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ImageManager::ThumbnailCache::instance()->insert(fileName, image);
    MainWindow::Window::theMainWindow()->reloadThumbnails();
}

DB::FileName UpdateVideoThumbnail::nextExistingImage(const DB::FileName &fileName, int frame, int direction)
{
    for (int i = 1; i < 10; ++i) {
        const int nextIndex = (frame + 10 + direction * i) % 10;
        const DB::FileName file = BackgroundJobs::HandleVideoThumbnailRequestJob::frameName(fileName, nextIndex);
        if (file.exists())
            return file;
    }
    Q_ASSERT(false && "We should always find at least the current frame");
    return DB::FileName();
}

} // namespace MainWindow
// vi:expandtab:tabstop=4 shiftwidth=4:
