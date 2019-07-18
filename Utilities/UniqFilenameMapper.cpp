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

#include "Utilities/UniqFilenameMapper.h"

#include <QFileInfo>

Utilities::UniqFilenameMapper::UniqFilenameMapper()
{
    /* nop */
}

Utilities::UniqFilenameMapper::UniqFilenameMapper(const QString &target)
    : m_targetDirectory(target)
{
    /* nop */
}

void Utilities::UniqFilenameMapper::reset()
{
    m_uniqFiles.clear();
    m_origToUniq.clear();
}

bool Utilities::UniqFilenameMapper::fileClashes(const QString &file)
{
    return m_uniqFiles.contains(file)
        || (!m_targetDirectory.isNull() && QFileInfo(file).exists());
}

QString Utilities::UniqFilenameMapper::uniqNameFor(const DB::FileName &filename)
{
    if (m_origToUniq.contains(filename))
        return m_origToUniq[filename];

    const QString extension = QFileInfo(filename.absolute()).completeSuffix();
    QString base = QFileInfo(filename.absolute()).baseName();
    if (!m_targetDirectory.isNull()) {
        base = QString::fromUtf8("%1/%2")
                   .arg(m_targetDirectory)
                   .arg(base);
    }

    QString uniqFile;
    int i = 0;
    do {
        uniqFile = (i == 0)
            ? QString::fromUtf8("%1.%2").arg(base).arg(extension)
            : QString::fromUtf8("%1-%2.%3").arg(base).arg(i).arg(extension);
        ++i;
    } while (fileClashes(uniqFile));

    m_origToUniq.insert(filename, uniqFile);
    m_uniqFiles.insert(uniqFile);
    return uniqFile;
}
// vi:expandtab:tabstop=4 shiftwidth=4:
