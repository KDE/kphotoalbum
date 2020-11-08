/* SPDX-FileCopyrightText: 2012-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
    connect(m_timer, &QTimer::timeout, this, &StatusIndicator::flicker);
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
        newColor = palette().mid().color();
    } else if (JobManager::instance()->isPaused() && !JobManager::instance()->hasActiveJobs())
        newColor = QColor(Qt::yellow).lighter();
    else
        newColor = (color() == palette().mid().color() ? currentColor() : palette().mid().color());

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
