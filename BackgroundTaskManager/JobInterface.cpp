/* SPDX-FileCopyrightText: 2012-2019 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
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
    connect(this, &JobInterface::completed, this, &JobInterface::stop);
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

#include "moc_JobInterface.cpp"
