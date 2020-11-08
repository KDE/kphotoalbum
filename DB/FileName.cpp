/* SPDX-FileCopyrightText: 2012-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "FileName.h"

#include <Settings/SettingsData.h>
#include <Utilities/FileNameUtil.h>

#include <QFile>

DB::FileName::FileName()
    : m_isNull(true)
{
}

DB::FileName DB::FileName::fromAbsolutePath(const QString &fileName)
{
    const QString imageRoot = Utilities::stripEndingForwardSlash(Settings::SettingsData::instance()->imageDirectory()) + QLatin1String("/");
    if (!fileName.startsWith(imageRoot))
        return FileName();

    FileName res;
    res.m_isNull = false;
    res.m_absoluteFilePath = fileName;
    res.m_relativePath = fileName.mid(imageRoot.length());
    return res;
}

DB::FileName DB::FileName::fromRelativePath(const QString &fileName)
{
    Q_ASSERT(!fileName.startsWith(QChar::fromLatin1('/')));
    FileName res;
    res.m_isNull = false;
    res.m_relativePath = fileName;
    res.m_absoluteFilePath = Utilities::stripEndingForwardSlash(Settings::SettingsData::instance()->imageDirectory()) + QLatin1String("/") + fileName;
    return res;
}

QString DB::FileName::absolute() const
{
    Q_ASSERT(!isNull());
    return m_absoluteFilePath;
}

QString DB::FileName::relative() const
{
    Q_ASSERT(!m_isNull);
    return m_relativePath;
}

bool DB::FileName::isNull() const
{
    return m_isNull;
}

bool DB::FileName::operator==(const DB::FileName &other) const
{
    return m_isNull == other.m_isNull && m_relativePath == other.m_relativePath;
}

bool DB::FileName::operator!=(const DB::FileName &other) const
{
    return !(*this == other);
}

bool DB::FileName::operator<(const DB::FileName &other) const
{
    return relative() < other.relative();
}

bool DB::FileName::exists() const
{
    return QFile::exists(absolute());
}

DB::FileName::operator QUrl() const
{
    return QUrl::fromLocalFile(absolute());
}

uint DB::qHash(const DB::FileName &fileName)
{
    return qHash(fileName.relative());
}
// vi:expandtab:tabstop=4 shiftwidth=4:
