/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "FileNameUtil.h"

#include <Settings/SettingsData.h>

QString Utilities::stripEndingForwardSlash(const QString &fileName)
{
    static QString slash = QString::fromLatin1("/");
    if (fileName.endsWith(slash))
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
    static QString slash = QString::fromLatin1("/");
    return stripEndingForwardSlash(Settings::SettingsData::instance()->imageDirectory()) + slash + relativeName;
}

QString Utilities::imageFileNameToAbsolute(const QString &fileName)
{
    static QString slash = QString::fromLatin1("/");
    static QString fileslashslash = QString::fromLatin1("file://");
    if (fileName.startsWith(Settings::SettingsData::instance()->imageDirectory()))
        return fileName;
    else if (fileName.startsWith(fileslashslash))
        return imageFileNameToAbsolute(fileName.mid(7)); // 7 == length("file://")
    else if (fileName.startsWith(slash))
        return QString(); // Not within our image root
    else
        return absoluteImageFileName(fileName);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
