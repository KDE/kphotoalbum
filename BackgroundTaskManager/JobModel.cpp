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
#include <QPixmap>
#include <QPainter>
#include <KLed>
#include <QTime>
#include <QTimer>

namespace BackgroundTaskManager {

JobModel::JobModel(QObject *parent) :
    QAbstractTableModel(parent)
{
    connect( JobManager::instance(), SIGNAL(jobStarted(JobInterface*)), this, SLOT(jobStarted(JobInterface*)));
    connect( JobManager::instance(), SIGNAL(jobEnded(JobInterface*)), this, SLOT(jobEnded(JobInterface*)));

    // Make the current task blink
    QTimer* timer = new QTimer(this);
    timer->start(500);
    connect(timer, SIGNAL(timeout()), this, SLOT(reset()));
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
    return 4;
}

QVariant JobModel::data(const QModelIndex &index, int role) const
{
    if ( !index.isValid() )
        return QVariant();

    const int row = index.row();
    const int col = index.column();

    JobInfo* current = info(row);
    if ( !current )
        return QVariant();

    if ( role == Qt::DisplayRole ) {
        switch (col) {
        case IDCol: return current->jobIndex();
        case TitleCol:   return current->title();
        case DetailsCol: return current->details();
        case ElapsedCol: return current->elapsed();
        default: return QVariant();
        }
    }
    else if ( role == Qt::DecorationRole && col == TitleCol )
        return statusImage(current->state);
    else if ( role == Qt::TextAlignmentRole )
        return index.column() == IDCol ? Qt::AlignRight : Qt::AlignLeft;

    return QVariant();
}

QVariant JobModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ( orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();
    switch (section) {
    case IDCol: return i18n("ID");
    case TitleCol:   return i18n("Title");
    case DetailsCol: return i18n("Details");
    case ElapsedCol: return i18n("Elapsed");
    default: return QVariant();
    }
}

void JobModel::reset()
{
    QAbstractTableModel::reset();
}

void JobModel::jobEnded(JobInterface *job)
{
    m_previousJobs.append( new CompletedJobInfo(job) );
    reset();
}

void JobModel::jobStarted(JobInterface *job)
{
    connect( job, SIGNAL(changed()), this, SLOT(reset()));
    reset();
}

JobInfo* JobModel::info(int row) const
{
    if ( row < m_previousJobs.count() )
        return  m_previousJobs[row];

    row -= m_previousJobs.count();
    if ( row  < JobManager::instance()->activeJobCount() )
        return JobManager::instance()->activeJob(row);

    row -= JobManager::instance()->activeJobCount();
    Q_ASSERT( row < JobManager::instance()->futureJobCount() );
    return JobManager::instance()->futureJob(row);
}

QPixmap JobModel::statusImage(JobInfo::State state) const
{
    QColor color;
    if ( state == JobInfo::Running )
        color = ( QTime::currentTime().msec() < 500 ) ?  Qt::gray : Qt::green;
    else if ( state == JobInfo::Completed )
        color = Qt::red;
    else
        color = QColor(Qt::yellow).darker();

    KLed led;
    led.setColor(color);

    QPalette pal = led.palette();
    pal.setColor(QPalette::Window, Qt::white);
    led.setPalette(pal);

    return QPixmap::grabWidget(&led);
}

} // namespace BackgroundTaskManager
