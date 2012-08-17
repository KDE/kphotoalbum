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

#ifndef JOBINFO_H
#define JOBINFO_H
#include <QString>
#include <QObject>
#include <QElapsedTimer>
#include "Priority.h"

namespace BackgroundTaskManager {

class JobInfo :public QObject
{
    Q_OBJECT

public:
    JobInfo(BackgroundTaskManager::Priority priority);
    JobInfo( const JobInfo* other );
    virtual ~JobInfo();

    virtual QString title() const = 0;
    virtual QString details() const = 0;
    BackgroundTaskManager::Priority priority() const;

    enum State { NotStarted, Running, Completed };
    State state;

    QString elapsed() const;
    int jobIndex() const;

protected slots:
    void start();
    void stop();

signals:
    void changed() const;

private:
    BackgroundTaskManager::Priority m_priority;
    QElapsedTimer m_timer;
    uint m_elapsed;
    int m_jobIndex;
    static int m_jobCounter;
};

}
#endif // JOBINFO_H
