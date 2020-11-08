/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "UniqFilenameMapper.h"

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
