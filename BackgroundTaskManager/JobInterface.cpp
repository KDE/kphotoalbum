/* Copyright (C) 2012-2018 Jesper K. Pedersen <blackie@kde.org>

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

#include "JobInterface.h"

#include "JobManager.h"
#include "Logging.h"

/**
  \class BackgroundTaskManager::JobInterface
  \brief Interfaces for jobs to be executed using \ref BackgroundTaskManager::JobManager

  Each job must override \ref execute, and must emit the signal completed.
  Emitting the signal is crusial, as the JobManager will otherwise stall.
*/

BackgroundTaskManager::JobInterface::JobInterface(BackgroundTaskManager::Priority priority)
    : JobInfo(priority)
    , m_dependencies(0)
{
    qCDebug(BackgroundTaskManagerLog) << "Created Job #" << jobIndex();
    connect(this, SIGNAL(completed()), this, SLOT(stop()));
}

BackgroundTaskManager::JobInterface::~JobInterface()
{
}

void BackgroundTaskManager::JobInterface::start()
{
    qCDebug(BackgroundTaskManagerLog, "Starting Job (#%d): %s %s", jobIndex(), qPrintable(title()), qPrintable(details()));
    JobInfo::start();
    execute();
}

void BackgroundTaskManager::JobInterface::addDependency(BackgroundTaskManager::JobInterface *job)
{
    m_dependencies++;
    connect(job, SIGNAL(completed()), this, SLOT(dependedJobCompleted()));
}

void BackgroundTaskManager::JobInterface::dependedJobCompleted()
{
    m_dependencies--;
    if (m_dependencies == 0)
        BackgroundTaskManager::JobManager::instance()->addJob(this);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
