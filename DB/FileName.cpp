/* Copyright 2012 Jesper K. Pedersen <blackie@kde.org>
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "FileName.h"
#include "ImageDB.h"

#include <DB/ImageInfoList.h>
#include <Utilities/Util.h>
#include <Settings/SettingsData.h>

#include <QFile>

DB::FileName::FileName()
    : m_isNull(true)
{
}

DB::FileName DB::FileName::fromAbsolutePath(const QString &fileName)
{
    const QString imageRoot = Utilities::stripEndingForwardSlash( Settings::SettingsData::instance()->imageDirectory() ) + QLatin1String("/");
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
    res.m_absoluteFilePath = Utilities::stripEndingForwardSlash( Settings::SettingsData::instance()->imageDirectory() ) + QLatin1String("/") + fileName;
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

bool DB::FileName::operator ==(const DB::FileName &other) const
{
    return m_isNull == other.m_isNull && m_relativePath == other.m_relativePath;
}

bool DB::FileName::operator !=(const DB::FileName &other) const
{
    return !(*this == other);
}

bool DB::FileName::operator <(const DB::FileName &other) const
{
    return relative() < other.relative();
}

bool DB::FileName::exists() const
{
    return QFile::exists(absolute());
}

DB::ImageInfoPtr DB::FileName::info() const
{
    return ImageDB::instance()->info(*this);
}

uint DB::qHash( const DB::FileName& fileName )
{
    return qHash(fileName.relative());
}
// vi:expandtab:tabstop=4 shiftwidth=4:
