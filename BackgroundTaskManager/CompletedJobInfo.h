/* SPDX-FileCopyrightText: 2012 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef BACKGROUNDTASKS_COMPLETEDJOBINFO_H
#define BACKGROUNDTASKS_COMPLETEDJOBINFO_H

#include "JobInfo.h"
namespace BackgroundTaskManager
{

class CompletedJobInfo : public JobInfo
{
public:
    explicit CompletedJobInfo(JobInfo *other);
    QString title() const override;
    QString details() const override;

private:
    QString m_title;
    QString m_details;
};

} // namespace BackgroundTaskManager

#endif // BACKGROUNDTASKS_COMPLETEDJOBINFO_H
// vi:expandtab:tabstop=4 shiftwidth=4:
