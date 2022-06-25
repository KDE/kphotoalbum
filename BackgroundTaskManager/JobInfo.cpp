/* SPDX-FileCopyrightText: 2012 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "JobInfo.h"

#include <KLocalizedString>

namespace BackgroundTaskManager
{

int JobInfo::s_jobCounter = 0;

JobInfo::JobInfo(BackgroundTaskManager::Priority priority)
    : state(NotStarted)
    , m_priority(priority)
    , m_elapsed(0)
    , m_jobIndex(++s_jobCounter)
{
}

JobInfo::JobInfo(const JobInfo *other)
{
    m_priority = other->m_priority;
    state = other->state;
    m_elapsed = other->m_elapsed;
    m_jobIndex = other->m_jobIndex;
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
    if (state == Completed)
        time = m_elapsed;

    const int secs = time / 1000;
    const int part = (time % 1000) / 100;

    return QString::fromLatin1("%1.%2").arg(secs).arg(part);
}

int JobInfo::jobIndex() const
{
    return m_jobIndex;
}

}
// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_JobInfo.cpp"
