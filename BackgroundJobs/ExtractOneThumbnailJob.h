// SPDX-FileCopyrightText: 2012 - 2022 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2024 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef BACKGROUNDJOBS_EXTRACTONETHUMBNAILJOB_H
#define BACKGROUNDJOBS_EXTRACTONETHUMBNAILJOB_H

#include <BackgroundTaskManager/JobInterface.h>
#include <kpabase/FileName.h>

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

Q_SIGNALS:
    void frameAvailable(const DB::FileName &fileName, int index, const QImage &frame);

private Q_SLOTS:
    void frameLoaded(const QImage &);

private:
    DB::FileName m_fileName;
    int m_index;
    bool m_wasCanceled;
};

} // namespace BackgroundJobs

#endif // BACKGROUNDJOBS_EXTRACTONETHUMBNAILJOB_H
// vi:expandtab:tabstop=4 shiftwidth=4:
