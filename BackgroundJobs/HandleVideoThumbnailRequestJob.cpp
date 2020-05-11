/* Copyright 2012-2020 The KPhotoAlbum Development Team

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

#include "HandleVideoThumbnailRequestJob.h"

#include <ImageManager/ExtractOneVideoFrame.h>
#include <ImageManager/ImageClientInterface.h>
#include <ImageManager/ImageRequest.h>
#include <ImageManager/ThumbnailCache.h>
#include <MainWindow/FeatureDialog.h>
#include <Settings/SettingsData.h>
#include <ThumbnailView/CellGeometry.h>
#include <Utilities/ImageUtil.h>

#include <KCodecs>
#include <KLocalizedString>
#include <QCryptographicHash>
#include <QDir>
#include <QIcon>
#include <QImage>

namespace BackgroundJobs
{

HandleVideoThumbnailRequestJob::HandleVideoThumbnailRequestJob(ImageManager::ImageRequest *request, BackgroundTaskManager::Priority priority, ImageManager::ThumbnailCache *thumbnailCache)
    : BackgroundTaskManager::JobInterface(priority)
    , m_request(request)
    , m_thumbnailCache(thumbnailCache)
{
}

QString HandleVideoThumbnailRequestJob::title() const
{
    return i18n("Extract Video Thumbnail");
}

QString HandleVideoThumbnailRequestJob::details() const
{
    return m_request->databaseFileName().relative();
}

void HandleVideoThumbnailRequestJob::execute()
{
    QImage image(pathForRequest(m_request->fileSystemFileName()).absolute());
    if (!image.isNull())
        frameLoaded(image);
    else
        ImageManager::ExtractOneVideoFrame::extract(m_request->fileSystemFileName(), 0, this, SLOT(frameLoaded(QImage)));
}

void HandleVideoThumbnailRequestJob::frameLoaded(QImage image)
{
    if (image.isNull())
        image = brokenImage();
    saveFullScaleFrame(m_request->databaseFileName(), image);
    sendResult(image);
    emit completed();
}

void HandleVideoThumbnailRequestJob::saveFullScaleFrame(const DB::FileName &fileName, const QImage &image)
{
    Utilities::saveImage(pathForRequest(fileName), image, "JPEG");
}

DB::FileName HandleVideoThumbnailRequestJob::pathForRequest(const DB::FileName &fileName)
{
    QCryptographicHash md5(QCryptographicHash::Md5);
    md5.addData(fileName.absolute().toUtf8());
    return DB::FileName::fromRelativePath(QString::fromLatin1(".videoThumbnails/%2").arg(QString::fromUtf8(md5.result().toHex())));
}

DB::FileName HandleVideoThumbnailRequestJob::frameName(const DB::FileName &videoName, int frameNumber)
{
    return DB::FileName::fromRelativePath(pathForRequest(videoName).relative() + QLatin1String("-") + QString::number(frameNumber));
}

void HandleVideoThumbnailRequestJob::removeFullScaleFrame(const DB::FileName &fileName)
{
    QDir().remove(BackgroundJobs::HandleVideoThumbnailRequestJob::pathForRequest(fileName).absolute());
}

void HandleVideoThumbnailRequestJob::sendResult(QImage image)
{
    //if ( m_request->isRequestStillValid(m_request) ) {
    image = image.scaled(QSize(m_request->width(), m_request->height()), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if (m_request->isThumbnailRequest()) {
        m_thumbnailCache->insert(m_request->databaseFileName(), image);
    }
    m_request->setLoadedOK(!image.isNull());
    m_request->client()->pixmapLoaded(m_request, image);
    //}
}

QImage HandleVideoThumbnailRequestJob::brokenImage() const
{
    return QIcon::fromTheme(QString::fromUtf8("applications-multimedia")).pixmap(ThumbnailView::CellGeometry::preferredIconSize()).toImage();
}

} // namespace BackgroundJobs
// vi:expandtab:tabstop=4 shiftwidth=4:
