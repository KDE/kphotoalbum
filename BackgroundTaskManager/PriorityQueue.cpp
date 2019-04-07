/* Copyright 2012 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "PriorityQueue.h"
#include "Utilities/AlgorithmHelper.h"
#include <functional>

using namespace Utilities;
namespace BackgroundTaskManager {

PriorityQueue::PriorityQueue()
{
    m_jobs.resize(SIZE_OF_PRIORITY_QUEUE);
}


bool PriorityQueue::isEmpty() const
{
    return all_of( m_jobs, std::mem_fn(&QueueType::isEmpty) );
}

int PriorityQueue::count() const
{
    return sum( m_jobs, std::mem_fn(&QueueType::length ) );
}

void PriorityQueue::enqueue(JobInterface *job, Priority priority)
{
    m_jobs[priority].enqueue(job);
}

JobInterface *PriorityQueue::dequeue()
{
    for( QueueType& queue : m_jobs ) {
        if ( !queue.isEmpty() )
            return queue.dequeue();
    }
    Q_ASSERT( false && "Queue was empty");
    return nullptr;
}

JobInterface *PriorityQueue::peek(int index) const
{
    int offset = 0;
    for ( const QueueType& queue : m_jobs) {
        if ( index-offset < queue.count() )
            return queue[index-offset];
        else
            offset += queue.count();
    }
    Q_ASSERT( false && "index beyond queue");
    return nullptr;
}

bool PriorityQueue::hasForegroundTasks() const
{
    for (int i = 0; i < BackgroundTask; ++i ) {
        if (!m_jobs[i].isEmpty())
            return true;
    }
    return false;
}

} // namespace BackgroundTaskManager
// vi:expandtab:tabstop=4 shiftwidth=4:
