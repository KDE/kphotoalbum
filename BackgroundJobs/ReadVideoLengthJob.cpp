/* Copyright (C) 2012 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

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
    connect(extractor, SIGNAL(lengthFound(int)), this, SLOT(lengthFound(int)));
    connect(extractor, SIGNAL(unableToDetermineLength()), this, SLOT(unableToDetermineLength()));
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
    emit completed();
}

void BackgroundJobs::ReadVideoLengthJob::unableToDetermineLength()
{
    // PENDING(blackie) Should we mark these as trouble, so we don't try them over and over again?
    emit completed();
}
// vi:expandtab:tabstop=4 shiftwidth=4:
