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

#ifndef JOBMANAGER_H
#define JOBMANAGER_H

#include "JobInterface.h"
#include "PriorityQueue.h"

#include <QObject>

namespace BackgroundTaskManager
{
class JobManager : public QObject
{
    Q_OBJECT
public:
    void addJob(JobInterface *job);
    static JobManager *instance();
    int activeJobCount() const;
    JobInfo *activeJob(int index) const;
    int futureJobCount() const;
    JobInfo *futureJob(int index) const;
    bool isPaused() const;
    bool hasActiveJobs() const;
    void togglePaused();

signals:
    void jobStarted(JobInterface *job);
    void jobEnded(JobInterface *job);

private slots:
    void execute();
    void jobCompleted();

private:
    JobManager();
    static JobManager *s_instance;
    bool shouldExecute() const;

    int maxJobCount() const;

    bool m_isRunning;
    QList<JobInterface *> m_active;
    PriorityQueue m_queue;
    bool m_isPaused;
};

}

#endif // JOBMANAGER_H
// vi:expandtab:tabstop=4 shiftwidth=4:
