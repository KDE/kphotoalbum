/* SPDX-FileCopyrightText: 2012-2019 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "JobManager.h"

#include "JobInfo.h"

#include <ImageManager/AsyncLoader.h>

#include <QThread>

/**
  \class BackgroundTaskManager::JobManager
  \brief Engine for running background jobs

  This is the engine for running background jobs. Each job is a subclass of
  \ref BackgroundTaskManager::JobInterface. The jobs are added using \ref addJob.

  Currently the jobs are executed one after the other on the main thread, but down the road I
  imagine it will provide for running jobs on secondary threads. The jobs would need to
  indicate that that is a possibility.
*/

BackgroundTaskManager::JobManager *BackgroundTaskManager::JobManager::s_instance = nullptr;

BackgroundTaskManager::JobManager::JobManager()
    : m_isPaused(false)
{
}

bool BackgroundTaskManager::JobManager::shouldExecute() const
{
    return m_queue.hasForegroundTasks() || !m_isPaused;
}

int BackgroundTaskManager::JobManager::maxJobCount() const
{
    // See comment in ImageManager::AsyncLoader::init()
    // We will at least have one active background task at the time, as some of them
    // currently aren't that much for background stuff. The key example of this is generating video thumbnails.
    const int max = qMin(3, QThread::idealThreadCount());
    int count = qMax(1, max - ImageManager::AsyncLoader::instance()->activeCount() - 1);
    return count;
}

void BackgroundTaskManager::JobManager::execute()
{
    if (m_queue.isEmpty())
        return;

    if (!shouldExecute())
        return;

    while (m_active.count() < maxJobCount() && !m_queue.isEmpty()) {
        JobInterface *job = m_queue.dequeue();
        connect(job, &JobInterface::completed, this, &JobManager::jobCompleted);
        m_active.append(job);
        emit jobStarted(job);
        job->start();
    }
}

void BackgroundTaskManager::JobManager::addJob(BackgroundTaskManager::JobInterface *job)
{
    m_queue.enqueue(job, job->priority());
    execute();
}

BackgroundTaskManager::JobManager *BackgroundTaskManager::JobManager::instance()
{
    if (!s_instance)
        s_instance = new JobManager;
    return s_instance;
}

int BackgroundTaskManager::JobManager::activeJobCount() const
{
    return m_active.count();
}

BackgroundTaskManager::JobInfo *BackgroundTaskManager::JobManager::activeJob(int index) const
{
    if (index < m_active.count())
        return m_active[index];
    return nullptr;
}

int BackgroundTaskManager::JobManager::futureJobCount() const
{
    return m_queue.count();
}

BackgroundTaskManager::JobInfo *BackgroundTaskManager::JobManager::futureJob(int index) const
{
    return m_queue.peek(index);
}

bool BackgroundTaskManager::JobManager::isPaused() const
{
    return m_isPaused;
}

bool BackgroundTaskManager::JobManager::hasActiveJobs() const
{
    return !m_active.isEmpty();
}

void BackgroundTaskManager::JobManager::jobCompleted()
{
    JobInterface *job = qobject_cast<JobInterface *>(sender());
    Q_ASSERT(job);
    emit jobEnded(job);
    m_active.removeAll(job);
    job->deleteLater();
    execute();
}

void BackgroundTaskManager::JobManager::togglePaused()
{
    m_isPaused = !m_isPaused;
    execute();
}
// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_JobManager.cpp"
