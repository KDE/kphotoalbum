// SPDX-FileCopyrightText: 2012-2013 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2013-2024 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2022 Tobias Leupold <tl@stonemx.de>
// SPDX-FileCopyrightText: 2026 Randall Rude <rsquared42@proton.me>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef BACKGROUNDJOBS_HANDLEVIDEOTHUMBNAILREQUESTJOB_H
#define BACKGROUNDJOBS_HANDLEVIDEOTHUMBNAILREQUESTJOB_H

#include <BackgroundTaskManager/JobInterface.h>

#include <QImage>

namespace ImageManager
{
class ImageRequest;
class ThumbnailCache;
class VideoThumbnailCache;
}
namespace DB
{
class FileName;
}
class QImage;

namespace BackgroundJobs
{

class HandleVideoThumbnailRequestJob : public BackgroundTaskManager::JobInterface
{
    Q_OBJECT
public:
    // Note: this class assumes ownership of the supplied ImageRequest.
    explicit HandleVideoThumbnailRequestJob(ImageManager::ImageRequest *request, BackgroundTaskManager::Priority priority, ImageManager::ThumbnailCache *thumbnailCache, ImageManager::VideoThumbnailCache *videoThumbnailCache);
    ~HandleVideoThumbnailRequestJob();
    QString title() const override;
    QString details() const override;

protected:
    void execute() override;

private Q_SLOTS:
    void frameLoaded(QImage);

private:
    void sendResult(QImage image);
    QImage brokenImage() const;

    ImageManager::ImageRequest *m_request;
    ImageManager::ThumbnailCache *m_thumbnailCache;
    ImageManager::VideoThumbnailCache *m_videoThumbnailCache;
};

} // namespace BackgroundJobs

#endif // BACKGROUNDJOBS_HANDLEVIDEOTHUMBNAILREQUESTJOB_H
// vi:expandtab:tabstop=4 shiftwidth=4:
