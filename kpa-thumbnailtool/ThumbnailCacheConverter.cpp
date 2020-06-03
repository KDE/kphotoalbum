/* Copyright (C) 2020 The KPhotoAlbum development team

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
#include "ThumbnailCacheConverter.h"
#include "Logging.h"

#include <QDataStream>
#include <QFile>
#include <QLoggingCategory>
#include <QString>
#include <QTemporaryFile>

int KPAThumbnailTool::convertV5ToV4Cache(const QString &indexFilename)
{
    QFile indexFile { indexFilename };
    if (!indexFile.open(QIODevice::ReadOnly)) {
        qCWarning(MainLog) << "Could not open thumbnailindex file!";
        qCWarning(MainLog) << "Aborting...";
        return 1;
    }
    QDataStream stream { &indexFile };
    int version;
    stream >> version;
    if (version != 5) {
        qCWarning(MainLog) << "Thumbnailindex is not a version 5 file!";
        qCWarning(MainLog) << "Aborting...";
        return 1;
    }
    // skip dimensions
    stream.skipRawData(sizeof(int));

    QTemporaryFile newIndexFile;
    if (!newIndexFile.open()) {
        qCWarning(MainLog) << "Could not open temporary file for writing!";
        qCWarning(MainLog) << "Aborting...";
        return 1;
    }
    QDataStream newStream { &newIndexFile };
    constexpr int v4FileVersion = 4;
    newStream << v4FileVersion;
    int numBytes = 0;
    char buf[256];
    do {
        numBytes = stream.readRawData(buf, 256);
        newStream.writeRawData(buf, numBytes);
    } while (numBytes != 0);
    if (!indexFile.rename(QString::fromUtf8("%1.bak").arg(indexFilename))) {
        qCWarning(MainLog) << "Could not back up thumbnailindex file!";
        qCWarning(MainLog) << "Aborting...";
        return 1;
    }
    if (!newIndexFile.copy(indexFilename)) {
        qCWarning(MainLog) << "Could not emplace new thumbnailindex file!" << newIndexFile.errorString();
        return 1;
    }
    return 0;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
