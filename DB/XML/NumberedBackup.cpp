// SPDX-FileCopyrightText: 2005 - 2010 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2007 - 2008 Laurent Montel <montel@kde.org>
// SPDX-FileCopyrightText: 2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2012 - 2024 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2012 Miika Turkia <miika.turkia@gmail.com>
// SPDX-FileCopyrightText: 2020 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "NumberedBackup.h"

#include <kpabase/FileUtil.h>
#include <kpabase/Logging.h>
#include <kpabase/SettingsData.h>
#include <kpabase/UIDelegate.h>

#include <KLocalizedString>
#include <KZip>
#include <QDir>
#include <QRegExp>

DB::NumberedBackup::NumberedBackup(DB::UIDelegate &ui)
    : m_ui(ui)
{
}

void DB::NumberedBackup::makeNumberedBackup()
{
    deleteOldBackupFiles();

    const QString fileName = QStringLiteral("index.xml~%1~").arg(getMaxId() + 1, 4, 10, QLatin1Char('0'));

    if (!QFileInfo::exists(QStringLiteral("%1/index.xml").arg(Settings::SettingsData::instance()->imageDirectory())))
        return;

    if (Settings::SettingsData::instance()->compressBackup()) {
        const QString fileNameWithExt = fileName + QLatin1String(".zip");
        const QString fileAndDir = QStringLiteral("%1/%2").arg(Settings::SettingsData::instance()->imageDirectory(), fileNameWithExt);
        KZip zip(fileAndDir);
        if (!zip.open(QIODevice::WriteOnly)) {
            m_ui.error(DB::LogMessage { DBLog(), QStringLiteral("Error creating zip file %1").arg(fileAndDir) }, i18n("Error creating zip file %1", fileAndDir), i18n("Error Making Numbered Backup"));
            return;
        }

        if (!zip.addLocalFile(QStringLiteral("%1/index.xml").arg(Settings::SettingsData::instance()->imageDirectory()), fileName)) {
            m_ui.error(DB::LogMessage { DBLog(), QStringLiteral("Error writing file %1 to zip file %2").arg(fileName, fileAndDir) }, i18n("Error writing file %1 to zip file %2", fileName, fileAndDir), i18n("Error Making Numbered Backup"));
        }
        zip.close();
    } else {
        Utilities::copyOrOverwrite(QStringLiteral("%1/index.xml").arg(Settings::SettingsData::instance()->imageDirectory()),
                                   QStringLiteral("%1/%2").arg(Settings::SettingsData::instance()->imageDirectory(), fileName));
    }
}

int DB::NumberedBackup::getMaxId() const
{
    const QStringList files = backupFiles();
    int max = 0;
    for (QStringList::ConstIterator fileIt = files.constBegin(); fileIt != files.constEnd(); ++fileIt) {
        bool OK;
        max = qMax(max, idForFile(*fileIt, OK));
    }
    return max;
}

QStringList DB::NumberedBackup::backupFiles() const
{
    QDir dir(Settings::SettingsData::instance()->imageDirectory());
    return dir.entryList(QStringList() << QStringLiteral("index.xml~*~*"), QDir::Files);
}

int DB::NumberedBackup::idForFile(const QString &fileName, bool &OK) const
{
    QRegExp reg(QStringLiteral("index\\.xml~([0-9]+)~(.zip)?"));
    if (reg.exactMatch(fileName)) {
        OK = true;
        return reg.cap(1).toInt();
    } else {
        OK = false;
        return -1;
    }
}

void DB::NumberedBackup::deleteOldBackupFiles()
{
    int maxId = getMaxId();
    int maxBackupFiles = Settings::SettingsData::instance()->backupCount();
    if (maxBackupFiles == -1)
        return;

    QStringList files = backupFiles();

    for (QStringList::ConstIterator fileIt = files.constBegin(); fileIt != files.constEnd(); ++fileIt) {
        bool OK;
        int num = idForFile(*fileIt, OK);
        if (OK && num <= maxId + 1 - maxBackupFiles) {
            (QDir(Settings::SettingsData::instance()->imageDirectory())).remove(*fileIt);
        }
    }
}
// vi:expandtab:tabstop=4 shiftwidth=4:
