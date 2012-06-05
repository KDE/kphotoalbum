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
#include "ImageManager/VideoLengthExtractor.h"
#include <DB/ImageDB.h>
#include <MainWindow/DirtyIndicator.h>
#include <klocale.h>

/**
  \class BackgroundTasks::ReadVideoLengthJob
  \brief Read the length of a video file and writes that to the database
*/

BackgroundTasks::ReadVideoLengthJob::ReadVideoLengthJob(const DB::FileName &fileName)
    :m_fileName(fileName)
{
}

void BackgroundTasks::ReadVideoLengthJob::execute()
{
    ImageManager::VideoLengthExtractor* extractor = new ImageManager::VideoLengthExtractor(this);
    extractor->extract(m_fileName);
    connect(extractor, SIGNAL(lengthFound(int)), this, SLOT(lengthFound(int)));
    connect(extractor, SIGNAL(unableToDetermineLength()), this, SLOT(unableToDetermindLength()));
}

QString BackgroundTasks::ReadVideoLengthJob::data() const
{
    return m_fileName.relative();
}

QString BackgroundTasks::ReadVideoLengthJob::title() const
{
    return i18n("Read Video Length");
}

void BackgroundTasks::ReadVideoLengthJob::lengthFound(int length)
{
    DB::ImageInfoPtr info = DB::ImageDB::instance()->info(m_fileName);
    info->setVideoLength(length);
    MainWindow::DirtyIndicator::markDirty();
    emit completed();
}

void BackgroundTasks::ReadVideoLengthJob::unableToDetermindLength()
{
    // PENDING(blackie) Should we mark these as trouble, so we don't try them over and over again?
    emit completed();
}
