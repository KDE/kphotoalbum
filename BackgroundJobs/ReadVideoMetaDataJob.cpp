// SPDX-FileCopyrightText: 2012-2022 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2025 Randall Rude <rsquared42@proton.me>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ReadVideoMetaDataJob.h"

#include <BackgroundTaskManager/JobInfo.h>
#include <DB/ImageDB.h>
#include <ImageManager/VideoMetaDataExtractor.h>
#include <MainWindow/DirtyIndicator.h>
#include <kpabase/Logging.h>

#include <KLocalizedString>

#include <QFileInfo>

BackgroundJobs::ReadVideoMetaDataJob::ReadVideoMetaDataJob(const DB::FileName &fileName, BackgroundTaskManager::Priority priority)
    : JobInterface(priority)
    , m_fileName(fileName)
    , m_creationTimeCompleted(false)
    , m_lengthCompleted(false)
{
}

void BackgroundJobs::ReadVideoMetaDataJob::execute()
{
    ImageManager::VideoMetaDataExtractor *extractor = new ImageManager::VideoMetaDataExtractor(this);
    extractor->extract(m_fileName);
    connect(extractor, &ImageManager::VideoMetaDataExtractor::creationTimeFound, this, &ReadVideoMetaDataJob::creationTimeFound);
    connect(extractor, &ImageManager::VideoMetaDataExtractor::lengthFound, this, &ReadVideoMetaDataJob::lengthFound);
    connect(extractor, &ImageManager::VideoMetaDataExtractor::unableToDetermineCreationTime, this, &ReadVideoMetaDataJob::unableToDetermineCreationTime);
    connect(extractor, &ImageManager::VideoMetaDataExtractor::unableToDetermineLength, this, &ReadVideoMetaDataJob::unableToDetermineLength);
}

QString BackgroundJobs::ReadVideoMetaDataJob::title() const
{
    return i18n("Read Video Length");
}

QString BackgroundJobs::ReadVideoMetaDataJob::details() const
{
    return m_fileName.relative();
}

void BackgroundJobs::ReadVideoMetaDataJob::lengthFound(int length)
{
    m_lengthCompleted = true;
    DB::ImageInfoPtr info = DB::ImageDB::instance()->info(m_fileName);

    // Only mark dirty if it is required
    if (info->videoLength() != length) {
        info->setVideoLength(length);
        MainWindow::DirtyIndicator::markDirty();
    }

    checkCompleted();
}

void BackgroundJobs::ReadVideoMetaDataJob::creationTimeFound(QDateTime dateTime)
{
    m_creationTimeCompleted = true;
    DB::ImageInfoPtr info = DB::ImageDB::instance()->info(m_fileName);

    const auto newDateTime = DB::ImageDate(dateTime);
    const auto fileInfo = QFileInfo(m_fileName.absolute());
    auto lastModifiedTime = fileInfo.lastModified();

    // Zero any fraction of a second.  DB::FileInfo drops any fraction of a
    // second from the last modified time.
    const auto lastModified = DB::ImageDate(lastModifiedTime.addMSecs(-lastModifiedTime.time().msec()));

    // Don't override a user-configured datetime.
    if (info->date() == lastModified) {
        // Only mark dirty if it is required.
        if (info->date() != newDateTime) {
            info->setDate(newDateTime);
            MainWindow::DirtyIndicator::markDirty();
        }
    } else {
        qCDebug(ImageManagerLog) << "Not overriding user-configured date"
                                 << info->date().toString(true) << "with lastModified="
                                 << lastModified.toString(true) << "for" << m_fileName.relative();
    }

    checkCompleted();
}

void BackgroundJobs::ReadVideoMetaDataJob::unableToDetermineLength()
{
    m_lengthCompleted = true;
    // PENDING(blackie) Should we mark these as trouble, so we don't try them over and over again?
    checkCompleted();
}

void BackgroundJobs::ReadVideoMetaDataJob::unableToDetermineCreationTime()
{
    m_creationTimeCompleted = true;
    checkCompleted();
}

void BackgroundJobs::ReadVideoMetaDataJob::checkCompleted()
{
    if (m_creationTimeCompleted && m_lengthCompleted) {
        Q_EMIT completed();
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_ReadVideoMetaDataJob.cpp"
