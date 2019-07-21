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
