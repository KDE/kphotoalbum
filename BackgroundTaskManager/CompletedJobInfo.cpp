/* SPDX-FileCopyrightText: 2012 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "CompletedJobInfo.h"

namespace BackgroundTaskManager
{

CompletedJobInfo::CompletedJobInfo(JobInfo *other)
    : JobInfo(other)
{
    m_title = other->title();
    m_details = other->details();
}

QString CompletedJobInfo::title() const
{
    return m_title;
}

QString CompletedJobInfo::details() const
{
    return m_details;
}

} // namespace BackgroundTaskManager
// vi:expandtab:tabstop=4 shiftwidth=4:
