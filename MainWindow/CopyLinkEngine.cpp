// SPDX-FileCopyrightText: 2021 - 2023 Tobias Leupold <tl at stonemx dot de>
// SPDX-FileCopyrightText: 2024 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

// Local includes
#include "CopyLinkEngine.h"

// KDE includes
#include <KIO/CopyJob>
#include <KLocalizedString>

// Qt includes
#include <QDebug>
#include <QFileDialog>

namespace MainWindow
{

CopyLinkEngine::CopyLinkEngine(QObject *parent)
    : QObject(parent)
{
}

void CopyLinkEngine::selectTarget(QWidget *parent, const QList<QUrl> &files, Action action)
{
    const auto count = files.count();
    if (count == 0) {
        return;
    }

    if (m_lastTarget.isEmpty()) {
        m_lastTarget = files.first().adjusted(QUrl::RemoveFilename).path();
    }

    QString title;
    switch (action) {
    case Copy:
        title = i18ncp("@title:window", "Copy image to ...", "Copy images to ...", count);
        break;
    case Link:
        title = i18ncp("@title:window", "Link image to ...", "Link images to ...", count);
        break;
    }

    const auto targetDirectory = QUrl::fromLocalFile(QFileDialog::getExistingDirectory(parent, title, m_lastTarget, QFileDialog::ShowDirsOnly));

    if (targetDirectory.isEmpty()) {
        return;
    }
    m_lastTarget = targetDirectory.path();

    KIO::CopyJob *job = nullptr;
    switch (action) {
    case Copy:
        job = KIO::copy(files, targetDirectory);
        break;
    case Link:
        job = KIO::link(files, targetDirectory);
        break;
    default:
        Q_UNREACHABLE();
    }
    connect(job, &KIO::CopyJob::finished, job, &QObject::deleteLater);
}

}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_CopyLinkEngine.cpp"
