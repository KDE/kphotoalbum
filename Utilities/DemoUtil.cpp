// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2018-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2020 Tobias Leupold <tl@stonemx.de>
// SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "DemoUtil.h"

#include "FileUtil.h"

#include <MainWindow/Window.h>
#include <kpabase/Logging.h>

#include <KIO/DeleteJob>
#include <KJob>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QStandardPaths>
#include <QUrl>

namespace
{

void copyList(const QStringList &from, const QString &directoryTo)
{
    for (QStringList::ConstIterator it = from.constBegin(); it != from.constEnd(); ++it) {
        const QString destFile = directoryTo + QString::fromLatin1("/") + QFileInfo(*it).fileName();
        if (!QFileInfo::exists(destFile)) {
            const bool ok = Utilities::copyOrOverwrite(*it, destFile);
            if (!ok) {
                KMessageBox::error(nullptr, i18n("Unable to copy '%1' to '%2'.", *it, destFile), i18n("Error Running Demo"));
                exit(-1);
            }
        }
    }
}
}

QString Utilities::setupDemo()
{
    const QString demoDir = QString::fromLatin1("%1/kphotoalbum-demo-%2").arg(QDir::tempPath(), QString::fromLocal8Bit(qgetenv("LOGNAME")));
    QFileInfo fi(demoDir);
    if (!fi.exists()) {
        bool ok = QDir().mkdir(demoDir);
        if (!ok) {
            KMessageBox::error(nullptr, i18n("Unable to create folder '%1' needed for demo.", demoDir), i18n("Error Running Demo"));
            exit(-1);
        }
    }

    // index.xml
    const QString demoDB = QStandardPaths::locate(QStandardPaths::DataLocation, QString::fromLatin1("demo/index.xml"));
    if (demoDB.isEmpty()) {
        qCDebug(UtilitiesLog) << "No demo database in standard locations:" << QStandardPaths::standardLocations(QStandardPaths::DataLocation);
        exit(-1);
    }
    const QString configFile = demoDir + QString::fromLatin1("/index.xml");
    if (QFile::exists(configFile))
        return configFile;

    copyOrOverwrite(demoDB, configFile);

    // Images
    const QStringList kpaDemoDirs = QStandardPaths::locateAll(
        QStandardPaths::DataLocation,
        QString::fromLatin1("demo"),
        QStandardPaths::LocateDirectory);
    QStringList images;
    for (const QString &dir : kpaDemoDirs) {
        QDirIterator it(dir, QStringList() << QStringLiteral("*.jpg") << QStringLiteral("*.avi"));
        while (it.hasNext()) {
            images.append(it.next());
        }
    }
    copyList(images, demoDir);

    // CategoryImages
    QString catDir = demoDir + QString::fromLatin1("/CategoryImages");
    fi = QFileInfo(catDir);
    if (!fi.exists()) {
        bool ok = QDir().mkdir(catDir);
        if (!ok) {
            KMessageBox::error(nullptr, i18n("Unable to create folder '%1' needed for demo.", catDir), i18n("Error Running Demo"));
            exit(-1);
        }
    }

    const QStringList kpaDemoCatDirs = QStandardPaths::locateAll(
        QStandardPaths::DataLocation,
        QString::fromLatin1("demo/CategoryImages"),
        QStandardPaths::LocateDirectory);
    QStringList catImages;
    for (const QString &dir : kpaDemoCatDirs) {
        QDirIterator it(dir, QStringList() << QStringLiteral("*.jpg"));
        while (it.hasNext()) {
            catImages.append(it.next());
        }
    }
    copyList(catImages, catDir);

    return configFile;
}

void Utilities::deleteDemo()
{
    QString dir = QString::fromLatin1("%1/kphotoalbum-demo-%2").arg(QDir::tempPath(), QString::fromLocal8Bit(qgetenv("LOGNAME")));
    QUrl demoUrl = QUrl::fromLocalFile(dir);
    KJob *delDemoJob = KIO::del(demoUrl);
    KJobWidgets::setWindow(delDemoJob, MainWindow::Window::theMainWindow());
    delDemoJob->exec();
}

// vi:expandtab:tabstop=4 shiftwidth=4:
