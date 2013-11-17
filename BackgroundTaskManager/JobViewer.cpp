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

#include "JobViewer.h"
#include "ui_JobViewer.h"
#include "JobModel.h"
#include "JobManager.h"

namespace BackgroundTaskManager {

JobViewer::JobViewer(QWidget *parent) :
    KDialog(parent), ui( new Ui::JobViewer ), m_model( nullptr )
{
    // disable default buttons (Ok, Cancel):
    setButtons( None );
    ui->setupUi( mainWidget() );
    setWindowTitle(i18n("Background Job Viewer"));
    connect( ui->pause, SIGNAL(clicked()), this, SLOT(togglePause()));
    connect( ui->pushButton, SIGNAL(clicked()), this, SLOT(accept()));
}

void JobViewer::setVisible(bool b)
{
    if (b) {
        m_model = new JobModel(this);
        ui->view->setModel(m_model);
        updatePauseButton();
    }
    else {
        delete m_model;
        m_model = nullptr;
    }


    ui->view->setColumnWidth(0, 50);
    ui->view->setColumnWidth(1, 300);
    ui->view->setColumnWidth(2, 300);
    ui->view->setColumnWidth(3, 50);
    KDialog::setVisible(b);
}

void JobViewer::togglePause()
{
    JobManager::instance()->togglePaused();
    updatePauseButton();
}

void JobViewer::updatePauseButton()
{
    ui->pause->setText(JobManager::instance()->isPaused() ? i18n("Continue") : i18n("Pause"));
}

} // namespace BackgroundTaskManager
// vi:expandtab:tabstop=4 shiftwidth=4:
