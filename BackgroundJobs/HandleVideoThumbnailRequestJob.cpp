// SPDX-FileCopyrightText: 2012-2022 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2013-2024 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2016-2022 Tobias Leupold <tl@stonemx.de>
// SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-FileCopyrightText: 2026 Randall Rude <rsquared42@proton.me>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "HandleVideoThumbnailRequestJob.h"

#include "kpabase/ImageUtil.h"
#include <ImageManager/ExtractOneVideoFrame.h>
#include <ImageManager/ImageClientInterface.h>
#include <ImageManager/ImageRequest.h>
#include <MainWindow/FeatureDialog.h>
#include <ThumbnailView/CellGeometry.h>
#include <KLocalizedString>
#include <QCryptographicHash>
#include <QDir>
#include <QIcon>
#include <QImage>
#include <kpabase/ImageUtil.h>
#include <kpabase/SettingsData.h>
#include <kpathumbnails/ThumbnailCache.h>
#include <kpathumbnails/VideoThumbnailCache.h>

namespace BackgroundJobs
{

HandleVideoThumbnailRequestJob::HandleVideoThumbnailRequestJob(ImageManager::ImageRequest *request, BackgroundTaskManager::Priority priority, ImageManager::ThumbnailCache *thumbnailCache, ImageManager::VideoThumbnailCache *videoThumbnailCache)
    : BackgroundTaskManager::JobInterface(priority)
    , m_request(request)
    , m_thumbnailCache(thumbnailCache)
    , m_videoThumbnailCache(videoThumbnailCache)
{
}

HandleVideoThumbnailRequestJob::~HandleVideoThumbnailRequestJob()
{
    delete m_request;
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
    QImage stillFrame = m_videoThumbnailCache->lookupStillFrame(m_request->databaseFileName());
    if (!stillFrame.isNull()) {
        sendResult(stillFrame);
        Q_EMIT completed();
    } else {
        ImageManager::ExtractOneVideoFrame::extract(m_request->fileSystemFileName(), 0, this, SLOT(frameLoaded(QImage)));
    }
}

void HandleVideoThumbnailRequestJob::frameLoaded(QImage image)
{
    if (image.isNull())
        image = brokenImage();
    m_videoThumbnailCache->insertThumbnail(m_request->databaseFileName(), 0, image);
    sendResult(image);
    Q_EMIT completed();
}

void HandleVideoThumbnailRequestJob::sendResult(QImage image)
{
    // if ( m_request->isRequestStillValid(m_request) ) {
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

#include "moc_HandleVideoThumbnailRequestJob.cpp"
