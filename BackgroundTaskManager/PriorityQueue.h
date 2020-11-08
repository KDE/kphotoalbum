/* SPDX-FileCopyrightText: 2012 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef BACKGROUNDTASKMANAGER_PRIORITYQUEUE_H
#define BACKGROUNDTASKMANAGER_PRIORITYQUEUE_H

#include "Priority.h"

#include <QQueue>
#include <QVector>

namespace BackgroundTaskManager
{

class JobInterface;

class PriorityQueue
{
public:
    PriorityQueue();
    bool isEmpty() const;
    int count() const;
    void enqueue(JobInterface *job, Priority priority);
    JobInterface *dequeue();
    JobInterface *peek(int index) const;
    bool hasForegroundTasks() const;

private:
    typedef QQueue<JobInterface *> QueueType;
    QVector<QueueType> m_jobs;
};

} // namespace BackgroundTaskManager

#endif // BACKGROUNDTASKMANAGER_PRIORITYQUEUE_H
// vi:expandtab:tabstop=4 shiftwidth=4:
