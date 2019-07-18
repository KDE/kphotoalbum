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

#ifndef BACKGROUNDJOBS_EXTRACTONETHUMBNAILJOB_H
#define BACKGROUNDJOBS_EXTRACTONETHUMBNAILJOB_H

#include <BackgroundTaskManager/JobInterface.h>
#include <DB/FileName.h>

class QImage;

namespace BackgroundJobs
{

/**
  \brief \ref BackgroundTaskManager::JobInterface "background job" for extracting the length of a video file.
  \see \ref videothumbnails
*/
class ExtractOneThumbnailJob : public BackgroundTaskManager::JobInterface
{
    Q_OBJECT

public:
    ExtractOneThumbnailJob(const DB::FileName &fileName, int index, BackgroundTaskManager::Priority priority);
    void execute() override;
    QString title() const override;
    QString details() const override;
    int index() const;
    void cancel();

private slots:
    void frameLoaded(const QImage &);

private:
    DB::FileName frameName() const;

    DB::FileName m_fileName;
    int m_index;
    bool m_wasCanceled;
};

} // namespace BackgroundJobs

#endif // BACKGROUNDJOBS_EXTRACTONETHUMBNAILJOB_H
// vi:expandtab:tabstop=4 shiftwidth=4:
