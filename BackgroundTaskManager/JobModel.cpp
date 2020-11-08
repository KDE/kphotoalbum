/* SPDX-FileCopyrightText: 2012-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "JobModel.h"

#include "CompletedJobInfo.h"
#include "JobInfo.h"
#include "JobManager.h"

#include <KLed>
#include <KLocalizedString>
#include <QApplication>
#include <QBitmap>
#include <QPainter>
#include <QPixmap>
#include <QTime>
#include <QTimer>

namespace BackgroundTaskManager
{

JobModel::JobModel(QObject *parent)
    : QAbstractTableModel(parent)
    , blinkStateOn(true)
{
    connect(JobManager::instance(), SIGNAL(jobStarted(JobInterface *)), this, SLOT(jobStarted(JobInterface *)));
    connect(JobManager::instance(), SIGNAL(jobEnded(JobInterface *)), this, SLOT(jobEnded(JobInterface *)));

    // Make the current task blink
    QTimer *timer = new QTimer(this);
    timer->start(500);
    connect(timer, &QTimer::timeout, this, &JobModel::heartbeat);
}

JobModel::~JobModel()
{
    qDeleteAll(m_previousJobs);
}

int JobModel::rowCount(const QModelIndex &index) const
{
    if (index.isValid())
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
    if (!index.isValid())
        return QVariant();

    const int row = index.row();
    const int col = index.column();

    JobInfo *current = info(row);
    if (!current)
        return QVariant();

    if (role == Qt::DisplayRole) {
        switch (col) {
        case IDCol:
            return current->jobIndex();
        case TitleCol:
            return current->title();
        case DetailsCol:
            return current->details();
        case ElapsedCol:
            return current->elapsed();
        default:
            return QVariant();
        }
    } else if (role == Qt::DecorationRole && col == TitleCol)
        return statusImage(current->state);
    else if (role == Qt::TextAlignmentRole)
        return index.column() == IDCol ? Qt::AlignRight : Qt::AlignLeft;

    return QVariant();
}

QVariant JobModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();
    switch (section) {
    case IDCol:
        return i18nc("@title:column Background job id", "ID");
    case TitleCol:
        return i18nc("@title:column Background job title", "Title");
    case DetailsCol:
        return i18nc("@title:column Additional information on background job", "Details");
    case ElapsedCol:
        return i18nc("@title:column Elapsed time", "Elapsed");
    default:
        return QVariant();
    }
}

void JobModel::reset()
{
    // FIXME: this is just a stand-in replacement for a call to the deprecated
    //        QAbstractTableModel::reset();
    // fix this by replacing the calls to reset() using:
    //  beginInsertRows()
    //  beginRemoveRows()
    //  beginMoveRows()
    beginResetModel();
    endResetModel();
}

void JobModel::jobEnded(JobInterface *job)
{
    m_previousJobs.append(new CompletedJobInfo(job));
    reset();
}

void JobModel::jobStarted(JobInterface *job)
{
    connect(job, SIGNAL(changed()), this, SLOT(reset()));
    reset();
}

void JobModel::heartbeat()
{
    beginResetModel();
    blinkStateOn = !blinkStateOn;
    // optional improvement: emit dataChanged for running jobs only
    endResetModel();
}

JobInfo *JobModel::info(int row) const
{
    if (row < m_previousJobs.count())
        return m_previousJobs[row];

    row -= m_previousJobs.count();
    if (row < JobManager::instance()->activeJobCount())
        return JobManager::instance()->activeJob(row);

    row -= JobManager::instance()->activeJobCount();
    Q_ASSERT(row < JobManager::instance()->futureJobCount());
    return JobManager::instance()->futureJob(row);
}

QPixmap JobModel::statusImage(JobInfo::State state) const
{
    QColor color;
    if (state == JobInfo::Running)
        color = blinkStateOn ? Qt::green : qApp->palette().mid().color();
    else if (state == JobInfo::Completed)
        color = Qt::red;
    else
        color = QColor(Qt::yellow).darker();

    KLed led;
    led.setColor(color);

    QPixmap pixmap = led.grab();
    // creating the mask by heuristic is expensive, so do it only once:
    static QBitmap s_ledMask = pixmap.createHeuristicMask();
    pixmap.setMask(s_ledMask);

    return pixmap;
}

} // namespace BackgroundTaskManager
// vi:expandtab:tabstop=4 shiftwidth=4:
