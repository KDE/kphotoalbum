/* SPDX-FileCopyrightText: 2012-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef BACKGROUNDJOBS_HANDLEVIDEOTHUMBNAILREQUESTJOB_H
#define BACKGROUNDJOBS_HANDLEVIDEOTHUMBNAILREQUESTJOB_H

#include <BackgroundTaskManager/JobInterface.h>

#include <QImage>

namespace ImageManager
{
class ImageRequest;
class ThumbnailCache;
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
    explicit HandleVideoThumbnailRequestJob(ImageManager::ImageRequest *request, BackgroundTaskManager::Priority priority, ImageManager::ThumbnailCache *thumbnailCache);
    QString title() const override;
    QString details() const override;
    static void saveFullScaleFrame(const DB::FileName &fileName, const QImage &image);
    static DB::FileName pathForRequest(const DB::FileName &fileName);
    static DB::FileName frameName(const DB::FileName &videoName, int frameNumber);
    static void removeFullScaleFrame(const DB::FileName &fileName);

protected:
    void execute() override;

private slots:
    void frameLoaded(QImage);

private:
    void sendResult(QImage image);
    QImage brokenImage() const;

    ImageManager::ImageRequest *m_request;
    ImageManager::ThumbnailCache *m_thumbnailCache;
};

} // namespace BackgroundJobs

#endif // BACKGROUNDJOBS_HANDLEVIDEOTHUMBNAILREQUESTJOB_H
// vi:expandtab:tabstop=4 shiftwidth=4:
