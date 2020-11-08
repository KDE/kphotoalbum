/* SPDX-FileCopyrightText: 2012 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "PriorityQueue.h"

#include <Utilities/AlgorithmHelper.h>

#include <functional>

using namespace Utilities;
namespace BackgroundTaskManager
{

PriorityQueue::PriorityQueue()
{
    m_jobs.resize(SIZE_OF_PRIORITY_QUEUE);
}

bool PriorityQueue::isEmpty() const
{
    return all_of(m_jobs, std::mem_fn(&QueueType::isEmpty));
}

int PriorityQueue::count() const
{
    return sum(m_jobs, std::mem_fn(&QueueType::length));
}

void PriorityQueue::enqueue(JobInterface *job, Priority priority)
{
    m_jobs[priority].enqueue(job);
}

JobInterface *PriorityQueue::dequeue()
{
    for (QueueType &queue : m_jobs) {
        if (!queue.isEmpty())
            return queue.dequeue();
    }
    Q_ASSERT(false && "Queue was empty");
    return nullptr;
}

JobInterface *PriorityQueue::peek(int index) const
{
    int offset = 0;
    for (const QueueType &queue : m_jobs) {
        if (index - offset < queue.count())
            return queue[index - offset];
        else
            offset += queue.count();
    }
    Q_ASSERT(false && "index beyond queue");
    return nullptr;
}

bool PriorityQueue::hasForegroundTasks() const
{
    for (int i = 0; i < BackgroundTask; ++i) {
        if (!m_jobs[i].isEmpty())
            return true;
    }
    return false;
}

} // namespace BackgroundTaskManager
// vi:expandtab:tabstop=4 shiftwidth=4:
