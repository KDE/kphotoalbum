// SPDX-FileCopyrightText: 2012-2020 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2021-2026 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "FileName.h"

#include "FileNameUtil.h"
#include "Logging.h"
#include "SettingsData.h"

#include <QFile>
#include <QLoggingCategory>

DB::FileName::FileName()
    : m_isNull(true)
{
}

DB::FileName DB::FileName::fromAbsolutePath(const QString &fileName)
{
    const QString imageRoot = Utilities::stripEndingForwardSlash(Settings::SettingsData::instance()->imageDirectory()) + QLatin1String("/");
    if (fileName.isEmpty()) {
        qCWarning(DBLog) << "Absolute filename cannot be empty!";
        return {};
    }
    if (!fileName.startsWith(imageRoot)) {
        qCWarning(DBLog) << "Absolute filename is outside of image root:" << fileName;
        return {};
    }

    FileName res;
    res.m_isNull = false;
    res.m_absoluteFilePath = fileName;
    res.m_relativePath = fileName.mid(imageRoot.length());
    if (res.m_relativePath.isEmpty()) {
        qCWarning(DBLog) << "Relative filename cannot be empty! Absolute filename:" << fileName;
        return {};
    }
    return res;
}

DB::FileName DB::FileName::fromRelativePath(const QString &fileName)
{
    if (fileName.startsWith(QChar::fromLatin1('/'))) {
        qCWarning(DBLog) << "Relative filename cannot start with '/':" << fileName;
        return {};
    }
    FileName res;
    res.m_isNull = false;
    res.m_relativePath = fileName;
    res.m_absoluteFilePath = Utilities::stripEndingForwardSlash(Settings::SettingsData::instance()->imageDirectory()) + QLatin1String("/") + fileName;
    if (res.m_relativePath.isEmpty() || res.m_absoluteFilePath.isEmpty()) {
        qCWarning(DBLog) << "Relative or absolute filename cannot be empty!";
        return {};
    }
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

bool DB::FileName::isValid() const
{
    return !isNull();
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
    if (isNull()) {
        qCWarning(DBLog) << "FileName for comparison is null!";
        return true;
    }
    if (other.isNull()) {
        qCWarning(DBLog) << "FileName for comparison is null!";
        return false;
    }
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
    if (fileName.isNull())
        return qHash(QString());
    return qHash(fileName.relative());
}
// vi:expandtab:tabstop=4 shiftwidth=4:
