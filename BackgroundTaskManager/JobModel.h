/* SPDX-FileCopyrightText: 2012 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef BACKGROUNDTASKS_JOBMODEL_H
#define BACKGROUNDTASKS_JOBMODEL_H

#include "CompletedJobInfo.h"
#include "JobInfo.h"
#include "JobInterface.h"

#include <QAbstractTableModel>

namespace BackgroundTaskManager
{

class JobModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit JobModel(QObject *parent = nullptr);
    ~JobModel() override;
    int rowCount(const QModelIndex &) const override;
    int columnCount(const QModelIndex &) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

public slots:
    void reset();

private slots:
    void jobEnded(JobInterface *job);
    void jobStarted(JobInterface *job);

    /**
     * @brief heartbeat
     * Makes the running jobs blink.
     */
    void heartbeat();

private:
    enum Column { IDCol = 0,
                  TitleCol = 1,
                  DetailsCol = 2,
                  ElapsedCol = 3 };
    bool blinkStateOn;

    JobInfo *info(int row) const;
    QPixmap statusImage(JobInfo::State state) const;

    QList<CompletedJobInfo *> m_previousJobs;
};

} // namespace BackgroundTaskManager

#endif // BACKGROUNDTASKS_JOBMODEL_H
// vi:expandtab:tabstop=4 shiftwidth=4:
