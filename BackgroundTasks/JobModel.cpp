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

#include "JobModel.h"
#include "JobManager.h"

namespace BackgroundTasks {

JobModel::JobModel(QObject *parent) :
    QAbstractTableModel(parent)
{
    connect( JobManager::instance(), SIGNAL(jobStarted(JobInterface*)), this, SLOT(jobStarted(JobInterface*)));
    connect( JobManager::instance(), SIGNAL(jobEnded(JobInterface*)), this, SLOT(jobEnded(JobInterface*)));
}

int JobModel::rowCount(const QModelIndex& index) const
{
    if ( index.isValid())
        return 0;
    else
        return m_previousJobs.count();
}

int JobModel::columnCount(const QModelIndex &) const
{
    return 2;
}

QVariant JobModel::data(const QModelIndex &index, int role) const
{
    if ( !index.isValid() )
        return QVariant();

    const int row = index.row();
    const int col = index.column();

    if ( role == Qt::DisplayRole ) {
        if ( col == 0 )
            return m_previousJobs[row].title;
        else if (col == 1)
            return m_previousJobs[row].data;
        else
            return QVariant();
    }

    return QVariant();
}

void JobModel::jobEnded(JobInterface *job)
{
    m_previousJobs.append( PreviousJobData(job->title(), job->data()));
    reset();
}

void JobModel::jobStarted(JobInterface *job)
{
    reset();
}

} // namespace BackgroundTasks
