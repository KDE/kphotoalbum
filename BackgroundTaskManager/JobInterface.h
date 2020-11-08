/* SPDX-FileCopyrightText: 2012 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef JOBINTERFACE_H
#define JOBINTERFACE_H

#include "JobInfo.h"

#include <QObject>

namespace BackgroundTaskManager
{

class JobInterface : public JobInfo
{
    Q_OBJECT
public:
    explicit JobInterface(BackgroundTaskManager::Priority);
    ~JobInterface() override;
    void start();
    void addDependency(JobInterface *job);

protected:
    virtual void execute() = 0;

signals:
    void completed();

private slots:
    void dependedJobCompleted();

private:
    int m_dependencies;
};

}

#endif // JOBINTERFACE_H
// vi:expandtab:tabstop=4 shiftwidth=4:
