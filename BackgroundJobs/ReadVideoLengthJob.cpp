// SPDX-FileCopyrightText: 2012-2022 The KPhotoAlbum Development Team
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ReadVideoLengthJob.h"

#include <BackgroundTaskManager/JobInfo.h>
#include <DB/ImageDB.h>
#include <ImageManager/VideoLengthExtractor.h>
#include <MainWindow/DirtyIndicator.h>

#include <KLocalizedString>

BackgroundJobs::ReadVideoLengthJob::ReadVideoLengthJob(const DB::FileName &fileName, BackgroundTaskManager::Priority priority)
    : JobInterface(priority)
    , m_fileName(fileName)
{
}

void BackgroundJobs::ReadVideoLengthJob::execute()
{
    ImageManager::VideoLengthExtractor *extractor = new ImageManager::VideoLengthExtractor(this);
    extractor->extract(m_fileName);
    connect(extractor, &ImageManager::VideoLengthExtractor::lengthFound, this, &ReadVideoLengthJob::lengthFound);
    connect(extractor, &ImageManager::VideoLengthExtractor::unableToDetermineLength, this, &ReadVideoLengthJob::unableToDetermineLength);
}

QString BackgroundJobs::ReadVideoLengthJob::title() const
{
    return i18n("Read Video Length");
}

QString BackgroundJobs::ReadVideoLengthJob::details() const
{
    return m_fileName.relative();
}

void BackgroundJobs::ReadVideoLengthJob::lengthFound(int length)
{
    DB::ImageInfoPtr info = DB::ImageDB::instance()->info(m_fileName);
    // Only mark dirty if it is required
    if (info->videoLength() != length) {
        info->setVideoLength(length);
        MainWindow::DirtyIndicator::markDirty();
    }
    Q_EMIT completed();
}

void BackgroundJobs::ReadVideoLengthJob::unableToDetermineLength()
{
    // PENDING(blackie) Should we mark these as trouble, so we don't try them over and over again?
    Q_EMIT completed();
}
// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_ReadVideoLengthJob.cpp"
