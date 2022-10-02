// SPDX-FileCopyrightText: 2012-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef JOBINFO_H
#define JOBINFO_H
#include "Priority.h"

#include <QElapsedTimer>
#include <QObject>
#include <QString>

namespace BackgroundTaskManager
{

class JobInfo : public QObject
{
    Q_OBJECT

public:
    explicit JobInfo(BackgroundTaskManager::Priority priority);
    explicit JobInfo(const JobInfo *other);
    ~JobInfo() override;

    virtual QString title() const = 0;
    virtual QString details() const = 0;
    BackgroundTaskManager::Priority priority() const;

    enum State { NotStarted,
                 Running,
                 Completed };
    State state;

    QString elapsed() const;
    int jobIndex() const;

protected Q_SLOTS:
    void start();
    void stop();

Q_SIGNALS:
    void changed() const;

private:
    BackgroundTaskManager::Priority m_priority;
    QElapsedTimer m_timer;
    uint m_elapsed;
    int m_jobIndex;
    static int s_jobCounter;
};

}
#endif // JOBINFO_H
// vi:expandtab:tabstop=4 shiftwidth=4:
