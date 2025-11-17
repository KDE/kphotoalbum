// SPDX-FileCopyrightText: 2012-2022 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2025 Randall Rude <rsquared42@proton.me>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef READVIDEOMETADATAJOB_H
#define READVIDEOMETADATAJOB_H

#include <BackgroundTaskManager/JobInterface.h>
#include <kpabase/FileName.h>

namespace BackgroundJobs
{

/**
  \brief Attempts to read the length and creation time of a video file and writes that to the database.
  \see \ref videothumbnails
*/
class ReadVideoMetaDataJob : public BackgroundTaskManager::JobInterface
{
    Q_OBJECT

public:
    ReadVideoMetaDataJob(const DB::FileName &fileName, BackgroundTaskManager::Priority priority);
    void execute() override;
    QString title() const override;
    QString details() const override;

private Q_SLOTS:
    // These are emitted when valid metadata is extracted.
    void creationTimeFound(QDateTime dateTime);
    void lengthFound(int);

    // These are emitted when valid metadata cannot be extracted.
    void unableToDetermineCreationTime();
    void unableToDetermineLength();

private:
    DB::FileName m_fileName;

    // These are set to true when the corresponding metadata is extracted.
    bool m_creationTimeCompleted;
    bool m_lengthCompleted;

    // Emits completed if all metadata extraction is completed.
    void checkCompleted();
};

}
#endif // READVIDEOMETADATAJOB_H
// vi:expandtab:tabstop=4 shiftwidth=4:
