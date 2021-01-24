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

DB::FileName Utilities::fileNameFromUserData(const QString &fileName)
{
    const auto inputUrl = QUrl::fromUserInput(fileName, Settings::SettingsData::instance()->imageDirectory(), QUrl::AssumeLocalFile);
    if (!inputUrl.isLocalFile())
        return {};

    const auto inputFileName = inputUrl.toLocalFile();
    if (inputFileName.startsWith(QStringLiteral("/")))
        return DB::FileName::fromAbsolutePath(inputFileName);
    else
        return DB::FileName::fromRelativePath(inputFileName);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
