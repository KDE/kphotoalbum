// SPDX-FileCopyrightText: 2005-2010 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2007-2008 Laurent Montel <montel@kde.org>
// SPDX-FileCopyrightText: 2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2012-2024 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2012 Miika Turkia <miika.turkia@gmail.com>
// SPDX-FileCopyrightText: 2020-2024 Tobias Leupold <tl@stonemx.de>
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
#include <QRegularExpression>

DB::NumberedBackup::NumberedBackup(DB::UIDelegate &ui, const QString &xmlFileName)
    : m_ui(ui)
    , m_xmlFileInfo(xmlFileName)
{
}

void DB::NumberedBackup::makeNumberedBackup()
{
    deleteOldBackupFiles();

    if (!m_xmlFileInfo.exists())
        return;

    const QString fileName = QStringLiteral("%1~%2~").arg(m_xmlFileInfo.fileName()).arg(getMaxId() + 1, 4, 10, QLatin1Char('0'));
    const QDir dir = m_xmlFileInfo.dir();

    if (Settings::SettingsData::instance()->compressBackup()) {
        const QString zipName = fileName + QLatin1String(".zip");
        const QString zipPath = dir.filePath(zipName);
        KZip zip(zipPath);
        if (!zip.open(QIODevice::WriteOnly)) {
            m_ui.error(DB::LogMessage { DBLog(), QStringLiteral("Error creating zip file %1").arg(zipPath) }, i18n("Error creating zip file %1", zipPath), i18n("Error Making Numbered Backup"));
            return;
        }

        if (!zip.addLocalFile(m_xmlFileInfo.filePath(), fileName)) {
            m_ui.error(DB::LogMessage { DBLog(), QStringLiteral("Error writing file %1 to zip file %2").arg(fileName, zipPath) }, i18n("Error writing file %1 to zip file %2", fileName, zipPath), i18n("Error Making Numbered Backup"));
        }
        zip.close();
    } else {
        Utilities::copyOrOverwrite(m_xmlFileInfo.filePath(), dir.filePath(fileName));
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
    const QDir dir = m_xmlFileInfo.dir();
    return dir.entryList(QStringList() << QStringLiteral("%1~*~*").arg(m_xmlFileInfo.fileName()), QDir::Files);
}

int DB::NumberedBackup::idForFile(const QString &fileName, bool &OK) const
{
    QRegExp reg(QStringLiteral("%1~([0-9]+)~(.zip)?").arg(m_xmlFileInfo.fileName()));
    if (reg.exactMatch(fileName)) {
        OK = true;
        return match.captured(1).toInt();
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

    QDir dir = m_xmlFileInfo.dir();

    for (QStringList::ConstIterator fileIt = files.constBegin(); fileIt != files.constEnd(); ++fileIt) {
        bool OK;
        int num = idForFile(*fileIt, OK);
        if (OK && num <= maxId + 1 - maxBackupFiles) {
            dir.remove(*fileIt);
        }
    }
}
// vi:expandtab:tabstop=4 shiftwidth=4:
