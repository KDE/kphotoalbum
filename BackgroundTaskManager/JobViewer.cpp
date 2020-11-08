/* SPDX-FileCopyrightText: 2012-2019 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "JobViewer.h"

#include "JobManager.h"
#include "JobModel.h"

#include <KLocalizedString>
#include <QDialogButtonBox>
#include <QLayout>
#include <QPushButton>
#include <QTreeView>

BackgroundTaskManager::JobViewer::JobViewer(QWidget *parent)
    : QDialog(parent)
    , m_model(nullptr)
{
    setWindowTitle(i18nc("@title:window", "Background Job Viewer"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    m_treeView = new QTreeView;
    mainLayout->addWidget(m_treeView);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    m_pauseButton = buttonBox->addButton(i18n("Pause"), QDialogButtonBox::YesRole);
    buttonBox->addButton(QDialogButtonBox::Close);

    connect(m_pauseButton, &QPushButton::clicked, this, &JobViewer::togglePause);
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
