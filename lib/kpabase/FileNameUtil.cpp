// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "FileNameUtil.h"

#include "FileName.h"
#include "SettingsData.h"

QString Utilities::stripEndingForwardSlash(const QString &fileName)
{
    if (fileName.endsWith(QStringLiteral("/")))
        return fileName.left(fileName.length() - 1);
    else
        return fileName;
}

QString Utilities::relativeFolderName(const QString &fileName)
{
    static QChar slash = QChar::fromLatin1('/');
    int index = fileName.lastIndexOf(slash, -1);
    if (index == -1)
        return QString();
    else
        return fileName.left(index);
}

QString Utilities::absoluteImageFileName(const QString &relativeName)
{
    return stripEndingForwardSlash(Settings::SettingsData::instance()->imageDirectory()) + QStringLiteral("/") + relativeName;
}

QString Utilities::imageFileNameToAbsolute(const QString &fileName)
{
    if (fileName.startsWith(Settings::SettingsData::instance()->imageDirectory()))
        return fileName;
    else if (fileName.startsWith(QStringLiteral("file://")))
        return imageFileNameToAbsolute(fileName.mid(7)); // 7 == length("file://")
    else if (fileName.startsWith(QStringLiteral("/")))
        return QString(); // Not within our image root
    else
        return absoluteImageFileName(fileName);
}

DB::FileName Utilities::fileNameFromUserData(const QString &fileName)
{
    return DB::FileName::fromAbsolutePath(imageFileNameToAbsolute(fileName));
}

// vi:expandtab:tabstop=4 shiftwidth=4:
