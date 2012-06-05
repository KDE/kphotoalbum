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

#include <QObject>
#include "JobInterface.h"
#include <QQueue>

namespace BackgroundTasks
{

class JobManager : public QObject
{
    Q_OBJECT
public:
    void addJob(JobInterface*);
    static JobManager* instance();

signals:
    void started();
    void ended();
    void jobStarted(JobInterface* job);
    void jobEnded(JobInterface* job);

private slots:
    void execute();
    void jobCompleted();

private:
    JobManager();
    static JobManager* m_instance;

    bool m_isRunning;
    JobInterface* m_active;
    QQueue<JobInterface*> m_queue;
};

}

#endif // JOBMANAGER_H
