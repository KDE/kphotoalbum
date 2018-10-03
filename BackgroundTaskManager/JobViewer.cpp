/* Copyright 2012-2016 Jesper K. Pedersen <blackie@kde.org>

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

#include <QLayout>
#include <QTreeView>
#include <QDialogButtonBox>
#include <QPushButton>

#include <KLocalizedString>

#include "JobViewer.h"
#include "JobModel.h"
#include "JobManager.h"

BackgroundTaskManager::JobViewer::JobViewer(QWidget *parent) : QDialog(parent), m_model(nullptr)
{
    setWindowTitle(i18nc("@title:window", "Background Job Viewer"));

    QVBoxLayout* mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    m_treeView = new QTreeView;
    mainLayout->addWidget(m_treeView);

    QDialogButtonBox* buttonBox = new QDialogButtonBox;
    m_pauseButton = buttonBox->addButton(i18n("Pause"), QDialogButtonBox::YesRole);
    buttonBox->addButton(QDialogButtonBox::Close);

    connect(m_pauseButton, SIGNAL(clicked()), this, SLOT(togglePause()));
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::accept);

    mainLayout->addWidget(buttonBox);
}

void BackgroundTaskManager::JobViewer::setVisible(bool b)
{
    if (b) {
        m_model = new JobModel(this);
        m_treeView->setModel(m_model);
        updatePauseButton();
    } else {
        delete m_model;
        m_model = nullptr;
    }

    m_treeView->setColumnWidth(0, 50);
    m_treeView->setColumnWidth(1, 300);
    m_treeView->setColumnWidth(2, 300);
    m_treeView->setColumnWidth(3, 50);
    QDialog::setVisible(b);
}

void BackgroundTaskManager::JobViewer::togglePause()
{
    JobManager::instance()->togglePaused();
    updatePauseButton();
}

void BackgroundTaskManager::JobViewer::updatePauseButton()
{
    m_pauseButton->setText(JobManager::instance()->isPaused() ? i18n("Continue") : i18n("Pause"));
}

// vi:expandtab:tabstop=4 shiftwidth=4:
