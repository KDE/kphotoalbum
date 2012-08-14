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

#include "JobInfo.h"
#include <KLocale>

namespace BackgroundTaskManager {

JobInfo::JobInfo(BackgroundTaskManager::Priority priority)
    : state(NotStarted), m_priority(priority), m_elapsed(0)
{
}

JobInfo::JobInfo(const JobInfo *other)
{
    m_priority = other->m_priority;
    state = other->state;
    m_elapsed = other->m_elapsed;
}

JobInfo::~JobInfo()
{
}

Priority JobInfo::priority() const
{
    return m_priority;
}

void JobInfo::start()
{
    m_timer.start();
    state = Running;
}

void JobInfo::stop()
{
    m_elapsed = m_timer.elapsed();
    state = Completed;
}

QString JobInfo::elapsed() const
{
    if (state == NotStarted)
        return i18n("Not Started");

    qint64 time = m_timer.elapsed();
    if ( state == Completed )
        time = m_elapsed;

    const int secs = time / 1000;
    const int msecs = time % 1000;

    return QString::fromLatin1("%1.%2").arg(secs).arg(msecs,3,10,QLatin1Char('0'));
}
}
