// SPDX-FileCopyrightText: 2012 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

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

    QList<JobInterface *> m_active;
    PriorityQueue m_queue;
    bool m_isPaused;
};

}

#endif // JOBMANAGER_H
// vi:expandtab:tabstop=4 shiftwidth=4:
