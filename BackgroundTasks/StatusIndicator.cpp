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
#include <QTimer>
#include <QApplication>
#include "JobManager.h"
#include "JobViewer.h"

namespace BackgroundTasks {

StatusIndicator::StatusIndicator( QWidget* parent )
    : KLed( Qt::green, parent ), m_timer( new QTimer(this) ), m_jobViewer(0)
{
    connect( m_timer, SIGNAL(timeout()), this, SLOT(flicker()));
    setCursor(Qt::PointingHandCursor);
    connect( JobManager::instance(), SIGNAL(started()), this, SLOT(startFlicker()));
    connect( JobManager::instance(), SIGNAL(ended()), this, SLOT(stopFlicker()));
}

void StatusIndicator::mouseReleaseEvent(QMouseEvent*)
{
    if ( !m_jobViewer )
        m_jobViewer = new JobViewer;

    m_jobViewer->setVisible(!m_jobViewer->isVisible());
}

void StatusIndicator::flicker()
{
    setColor( color() == Qt::green ? Qt::gray : Qt::green );
}

void StatusIndicator::startFlicker()
{
    m_timer->start(500);
}

void StatusIndicator::stopFlicker()
{
    setColor( Qt::gray );
    m_timer->stop();
}

} // namespace BackgroundTasks
