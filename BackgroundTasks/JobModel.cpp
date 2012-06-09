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
#include "JobInfo.h"
#include <KLocale>
#include "CompletedJobInfo.h"

namespace BackgroundTasks {

JobModel::JobModel(QObject *parent) :
    QAbstractTableModel(parent)
{
    connect( JobManager::instance(), SIGNAL(jobStarted(JobInterface*)), this, SLOT(jobStarted(JobInterface*)));
    connect( JobManager::instance(), SIGNAL(jobEnded(JobInterface*)), this, SLOT(jobEnded(JobInterface*)));
}

JobModel::~JobModel()
{
    qDeleteAll(m_previousJobs);
}

int JobModel::rowCount(const QModelIndex& index) const
{
    if ( index.isValid())
        return 0;
    else
        return m_previousJobs.count() + JobManager::instance()->activeJobCount() + JobManager::instance()->futureJobCount();
}

int JobModel::columnCount(const QModelIndex &) const
{
    return 3;
}

QVariant JobModel::data(const QModelIndex &index, int role) const
{
    if ( !index.isValid() )
        return QVariant();

    const int row = index.row();
    const int col = index.column();

    JobInfo* inf = info(row);

    if ( role == Qt::DisplayRole ) {
        switch (col) {
        case ActiveCol:  return type(inf->jobType);
        case TitleCol:   return inf->title();
        case DetailsCol: return inf->details();
        default: return QVariant();
        }
    }

    return QVariant();
}

QVariant JobModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ( orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();
    switch (section) {
    case ActiveCol:  return i18n("Status");
    case TitleCol:   return i18n("Title");
    case DetailsCol: return i18n("Details");
    default: return QVariant();
    }
}

void JobModel::jobEnded(JobInterface *job)
{
    m_previousJobs.append( new CompletedJobInfo(job) );
    reset();
}

void JobModel::jobStarted(JobInterface *job)
{
    Q_UNUSED(job);
    reset();
}

JobInfo* JobModel::info(int row) const
{
    if ( row < m_previousJobs.count() ) {
        JobInfo* result = m_previousJobs[row];
        result->jobType = JobInfo::PastJob;
        return result;
    }

    row -= m_previousJobs.count();
    if ( row  < JobManager::instance()->activeJobCount() ) {
        JobInfo* result = JobManager::instance()->activeJob(row);
        result->jobType = JobInfo::CurrentJob;
        return result;
    }

    row -= JobManager::instance()->activeJobCount();
    Q_ASSERT( row < JobManager::instance()->futureJobCount() );
    JobInfo* result = JobManager::instance()->futureJob(row);
    result->jobType = JobInfo::FutureJob;
    return result;
}

QString JobModel::type(JobInfo::JobType type) const
{
    switch (type) {
    case JobInfo::PastJob: return i18n("Past");
    case JobInfo::CurrentJob: return i18n("Active");
    case JobInfo::FutureJob: return i18n("Future");
    }
    return QString();
}

} // namespace BackgroundTasks
