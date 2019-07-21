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

#include "StatusIndicator.h"

#include "JobManager.h"
#include "JobViewer.h"

#include <KLocalizedString>
#include <QApplication>
#include <QHelpEvent>
#include <QTimer>
#include <QToolTip>

namespace BackgroundTaskManager
{

StatusIndicator::StatusIndicator(QWidget *parent)
    : KLed(Qt::green, parent)
    , m_timer(new QTimer(this))
    , m_jobViewer(nullptr)
{
    connect(m_timer, SIGNAL(timeout()), this, SLOT(flicker()));
    setCursor(Qt::PointingHandCursor);
    connect(JobManager::instance(), SIGNAL(jobStarted(JobInterface *)), this, SLOT(maybeStartFlicker()));
}

bool StatusIndicator::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {
        showToolTip(dynamic_cast<QHelpEvent *>(event));
        return true;
    }
    return KLed::event(event);
}

void StatusIndicator::mouseReleaseEvent(QMouseEvent *)
{
    if (!m_jobViewer)
        m_jobViewer = new JobViewer;

    m_jobViewer->setVisible(!m_jobViewer->isVisible());
}

void StatusIndicator::flicker()
{
    QColor newColor;

    if (!JobManager::instance()->hasActiveJobs()) {
        m_timer->stop();
        newColor = Qt::gray;
    } else if (JobManager::instance()->isPaused() && !JobManager::instance()->hasActiveJobs())
        newColor = QColor(Qt::yellow).lighter();
    else
        newColor = (color() == Qt::gray ? currentColor() : Qt::gray);

    setColor(newColor);
}

void StatusIndicator::maybeStartFlicker()
{
    if (!m_timer->isActive())
        m_timer->start(500);
}

QColor StatusIndicator::currentColor() const
{
    return JobManager::instance()->isPaused() ? Qt::yellow : Qt::green;
}

void StatusIndicator::showToolTip(QHelpEvent *event)
{
    const int activeCount = JobManager::instance()->activeJobCount();
    const int pendingCount = JobManager::instance()->futureJobCount();

    const QString text = i18n("<p>Active jobs: %1<br/>"
                              "Pending jobs: %2"
                              "<hr/><br/>"
                              "Color codes:"
                              "<ul><li><b>blinking green</b>: Active background jobs</li>"
                              "<li><b>gray</b>: No active jobs</li>"
                              "<li><b>solid yellow</b>: Job queue is paused</li>"
                              "<li><b>blinking yellow</b>: Job queue is paused for background jobs, but is executing a foreground job "
                              "(like extracting a thumbnail for a video file, which is currently shown in the thumbnail viewer)</li></ul></p>",
                              activeCount, pendingCount);
    QToolTip::showText(event->globalPos(), text);
}

} // namespace BackgroundTaskManager
// vi:expandtab:tabstop=4 shiftwidth=4:
