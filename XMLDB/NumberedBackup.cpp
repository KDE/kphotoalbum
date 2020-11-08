/* SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "NumberedBackup.h"

#include <DB/UIDelegate.h>
#include <Settings/SettingsData.h>
#include <Utilities/FileUtil.h>

#include <KLocalizedString>
#include <KZip>
#include <QDir>
#include <QRegExp>

XMLDB::NumberedBackup::NumberedBackup(DB::UIDelegate &ui)
    : m_ui(ui)
{
}

void XMLDB::NumberedBackup::makeNumberedBackup()
{
    deleteOldBackupFiles();

    const QString fileName = QStringLiteral("index.xml~%1~").arg(getMaxId() + 1, 4, 10, QLatin1Char('0'));

    if (!QFileInfo(QStringLiteral("%1/index.xml").arg(Settings::SettingsData::instance()->imageDirectory())).exists())
        return;

    if (Settings::SettingsData::instance()->compressBackup()) {
        const QString fileNameWithExt = fileName + QLatin1String(".zip");
        const QString fileAndDir = QStringLiteral("%1/%2").arg(Settings::SettingsData::instance()->imageDirectory()).arg(fileNameWithExt);
        KZip zip(fileAndDir);
        if (!zip.open(QIODevice::WriteOnly)) {
            m_ui.error(QStringLiteral("Error creating zip file %1").arg(fileAndDir), i18n("Error creating zip file %1", fileAndDir), i18n("Error Making Numbered Backup"));
            return;
        }

        if (!zip.addLocalFile(QStringLiteral("%1/index.xml").arg(Settings::SettingsData::instance()->imageDirectory()), fileName)) {
            m_ui.error(QStringLiteral("Error writing file %1 to zip file %2").arg(fileName).arg(fileAndDir), i18n("Error writing file %1 to zip file %2", fileName, fileAndDir), i18n("Error Making Numbered Backup"));
        }
        zip.close();
    } else {
        Utilities::copyOrOverwrite(QStringLiteral("%1/index.xml").arg(Settings::SettingsData::instance()->imageDirectory()),
                                   QStringLiteral("%1/%2").arg(Settings::SettingsData::instance()->imageDirectory()).arg(fileName));
    }
}

int XMLDB::NumberedBackup::getMaxId() const
{
    const QStringList files = backupFiles();
    int max = 0;
    for (QStringList::ConstIterator fileIt = files.constBegin(); fileIt != files.constEnd(); ++fileIt) {
        bool OK;
        max = qMax(max, idForFile(*fileIt, OK));
    }
    return max;
}

QStringList XMLDB::NumberedBackup::backupFiles() const
{
    QDir dir(Settings::SettingsData::instance()->imageDirectory());
    return dir.entryList(QStringList() << QStringLiteral("index.xml~*~*"), QDir::Files);
}

int XMLDB::NumberedBackup::idForFile(const QString &fileName, bool &OK) const
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

void XMLDB::NumberedBackup::deleteOldBackupFiles()
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
