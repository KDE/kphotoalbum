/* SPDX-FileCopyrightText: 2014-2019 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/

// Qt includes
#include <QFileDialog>
#include <QPushButton>

// KDE includes
#include <KIO/CopyJob>
#include <KLocalizedString>

// Local includes
#include "CopyPopup.h"

MainWindow::CopyPopup::CopyPopup(QWidget *parent,
                                 QUrl &selectedFile,
                                 QList<QUrl> &allSelectedFiles,
                                 QString &lastTarget,
                                 CopyType copyType)
    : QMenu(parent)
    , m_selectedFile(selectedFile)
    , m_allSelectedFiles(allSelectedFiles)
    , m_lastTarget(lastTarget)
{
    connect(this, &CopyPopup::triggered, this, &CopyPopup::slotCopy);
    QAction *action;

    bool haveSeveralFiles = m_allSelectedFiles.size() > 1;
    switch (copyType) {
    case Copy:
        setTitle(i18n("Copy image(s) to..."));
        action = addAction(i18n("Copy currently selected image to..."));
        action->setData(CopyAction::CopyCurrent);
        if (m_selectedFile.isEmpty())
            action->setEnabled(false);
        action = addAction(i18n("Copy all selected images to..."));
        action->setData(CopyAction::CopyAll);
        action->setEnabled(haveSeveralFiles);
        break;

    case Link:
        action = addAction(i18n("Link currently selected image to..."));
        setTitle(i18n("Link image(s) to..."));
        action->setData(CopyAction::LinkCurrent);
        if (m_selectedFile.isEmpty())
            action->setEnabled(false);
        action = addAction(i18n("Link all selected images to..."));
        action->setData(CopyAction::LinkAll);
        action->setEnabled(haveSeveralFiles);
        break;
    }
}

void MainWindow::CopyPopup::slotCopy(QAction *action)
{
    if (m_lastTarget.isNull()) {
        m_lastTarget = m_allSelectedFiles.at(0).adjusted(QUrl::RemoveFilename).path();
    }

    CopyAction copyAction = static_cast<CopyAction>(action->data().toUInt());
    QFileDialog dialog(this);

    if (copyAction == CopyCurrent || copyAction == LinkCurrent) {

        if (copyAction == CopyCurrent) {
            dialog.setWindowTitle(i18nc("@title:window", "Copy Image to..."));
            dialog.setLabelText(QFileDialog::Accept, i18nc("@action:button", "Copy"));
        } else if (copyAction == LinkCurrent) {
            dialog.setWindowTitle(i18nc("@title:window", "Link Image to..."));
            dialog.setLabelText(QFileDialog::Accept, i18nc("@action:button", "Link"));
        }
        dialog.setDirectory(m_lastTarget);
        dialog.selectFile(m_selectedFile.fileName());
        dialog.setAcceptMode(QFileDialog::AcceptSave);

        if (dialog.exec()) {
            QUrl target = dialog.selectedUrls().first();
            m_lastTarget = target.adjusted(QUrl::RemoveFilename).path();

            KIO::CopyJob *job;
            if (copyAction == CopyCurrent) {
                job = KIO::copy(m_selectedFile, target);
            } else if (copyAction == LinkCurrent) {
                job = KIO::link(m_selectedFile, target);
            }
            connect(job, &KIO::CopyJob::finished, job, &QObject::deleteLater);
        }

    } else if (copyAction == CopyAll || copyAction == LinkAll) {

        QString title;
        if (copyAction == CopyAll) {
            title = i18nc("@title:window", "Copy images to...");
        } else if (copyAction == LinkAll) {
            title = i18nc("@title:window", "Link images to...");
        }

        QString target = QFileDialog::getExistingDirectory(this, title, m_lastTarget,
                                                           QFileDialog::ShowDirsOnly);

        if (!target.isEmpty()) {
            m_lastTarget = target;
            QUrl targetUrl = QUrl::fromLocalFile(target);

            KIO::CopyJob *job;
            if (copyAction == CopyAll) {
                job = KIO::copy(m_allSelectedFiles, targetUrl);
            } else if (copyAction == LinkAll) {
                job = KIO::link(m_allSelectedFiles, targetUrl);
            }
            connect(job, &KIO::CopyJob::finished, job, &QObject::deleteLater);
        }
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:
