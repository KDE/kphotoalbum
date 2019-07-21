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

#ifndef BACKGROUNDJOBS_HANDLEVIDEOTHUMBNAILREQUESTJOB_H
#define BACKGROUNDJOBS_HANDLEVIDEOTHUMBNAILREQUESTJOB_H

#include <BackgroundTaskManager/JobInterface.h>

#include <QImage>

namespace ImageManager
{
class ImageRequest;
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
    explicit HandleVideoThumbnailRequestJob(ImageManager::ImageRequest *request, BackgroundTaskManager::Priority priority);
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
};

} // namespace BackgroundJobs

#endif // BACKGROUNDJOBS_HANDLEVIDEOTHUMBNAILREQUESTJOB_H
// vi:expandtab:tabstop=4 shiftwidth=4:
