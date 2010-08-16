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

Utilities::UniqFilenameMapper::UniqFilenameMapper() {
    /* nop */
}

Utilities::UniqFilenameMapper::UniqFilenameMapper(const QString &target)
    : _targetDirectory(target) {
    /* nop */
}


void Utilities::UniqFilenameMapper::reset() {
    _uniqFiles.clear();
    _origToUniq.clear();
}

bool Utilities::UniqFilenameMapper::fileClashes(const QString &file) {
    return _uniqFiles.contains(file)
        || (!_targetDirectory.isNull() && QFileInfo(file).exists());
}

QString Utilities::UniqFilenameMapper::uniqNameFor(const QString& filename) {
    if (_origToUniq.contains(filename))
        return _origToUniq[filename];

    const QString extension = QFileInfo(filename).completeSuffix();
    QString base = QFileInfo(filename).baseName();
    if (!_targetDirectory.isNull()) {
        base = QString::fromAscii("%1/%2")
            .arg(_targetDirectory).arg(base);
    }

    QString uniqFile;
    int i = 0;
    do {
        uniqFile = (i == 0)
            ? QString::fromAscii("%1.%2").arg(base).arg(extension)
            : QString::fromAscii("%1-%2.%3").arg(base).arg(i).arg(extension);
        ++i;
    }
    while (fileClashes(uniqFile));

    _origToUniq.insert(filename, uniqFile);
    _uniqFiles.insert(uniqFile);
    return uniqFile;
}
