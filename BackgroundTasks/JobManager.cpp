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

#include "JobManager.h"
#include "JobInfo.h"

/**
  \class BackgroundTasks::JobManager
  \breif Engine for running background jobs

  This is the engine for running background jobs. Each job is a subclass of
  \ref BackgroundTasks::JobInterface. The jobs are added using \ref addJob.

  Currently the jobs are executed one after the other on the main thread, but down the road I
  imagine it will provide for running jobs on secondary threads. The jobs would need to
  indicate that that is a possibility.
*/

BackgroundTasks::JobManager* BackgroundTasks::JobManager::m_instance = 0;

BackgroundTasks::JobManager::JobManager() :
    m_isRunning(false), m_active(0)
{
}

void BackgroundTasks::JobManager::execute()
{
    if ( m_queue.isEmpty() ) {
        m_isRunning = false;
        emit ended();
        return;
    }

    m_isRunning = true;
    emit started();

    m_active = m_queue.dequeue();
    connect(m_active,SIGNAL(completed()), this, SLOT(jobCompleted()));
    m_active->execute();
    emit jobStarted(m_active);
}

void BackgroundTasks::JobManager::addJob(BackgroundTasks::JobInterface* job )
{
    m_queue.enqueue(job);
    if (!m_isRunning)
        execute();
}

BackgroundTasks::JobManager *BackgroundTasks::JobManager::instance()
{
    if ( !m_instance )
        m_instance = new JobManager;
    return m_instance;
}

int BackgroundTasks::JobManager::activeJobCount() const
{
    if ( m_active )
        return 1;
    else
        return 0;
}

BackgroundTasks::JobInfo BackgroundTasks::JobManager::activeJob(int index) const
{
    // Pending extend to support multiple jobs
    Q_UNUSED(index);
    return m_active->info();
}

int BackgroundTasks::JobManager::futureJobCount() const
{
    return m_queue.count();
}

BackgroundTasks::JobInfo BackgroundTasks::JobManager::futureJob(int index) const
{
    return m_queue[index]->info();
}

void BackgroundTasks::JobManager::jobCompleted()
{
    emit jobEnded(m_active);
    delete m_active;
    m_active = 0;
    execute();
}
