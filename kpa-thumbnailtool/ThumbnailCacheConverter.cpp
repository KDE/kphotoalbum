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

#include <KLocalizedString>
#include <QDataStream>
#include <QFile>
#include <QString>
#include <QTemporaryFile>
#include <QTextStream>

int KPAThumbnailTool::convertV5ToV4Cache(const QString &indexFilename, QTextStream &err)
{
    QFile fromFile { indexFilename };
    if (!fromFile.open(QIODevice::ReadOnly)) {
        err << i18nc("@info:shell", "Could not open file `thumbnailindex`!\n");
        err << i18nc("@info:shell", "Thumbnail index was not changed.\n");
        return 1;
    }
    QTemporaryFile toFile;
    if (!toFile.open()) {
        err << i18nc("@info:shell", "Could not open temporary file for writing!\n");
        err << i18nc("@info:shell", "Thumbnail index was not changed.\n");
        return 1;
    }

    if (!convertV5ToV4Cache(fromFile, toFile, err))
        return 1;

    if (!fromFile.rename(QString::fromUtf8("%1.bak").arg(indexFilename))) {
        err << i18nc("@info:shell", "Could not back up file `thumbnailindex`!\n");
        err << i18nc("@info:shell", "Thumbnail index was not changed.\n");
        return 1;
    }
    if (!toFile.copy(indexFilename)) {
        err << i18nc("@info:shell", "Could not copy temporary thumbnail index file to final location!\n");
        err << i18nc("@info:shell", "Error message was: %1\n", toFile.errorString());
        return 1;
    }
    return 0;
}

bool KPAThumbnailTool::convertV5ToV4Cache(QIODevice &fromFile, QIODevice &toFile, QTextStream &err)
{
    QDataStream fromStream { &fromFile };
    int version;
    fromStream >> version;
    if (version != 5) {
        err << i18nc("@info:shell", "Thumbnail index is not a version 5 file!\n");
        err << i18nc("@info:shell", "Thumbnail index was not changed.\n");
        return false;
    }
    // skip dimensions
    fromStream.skipRawData(sizeof(int));

    QDataStream toStream { &toFile };
    constexpr int v4FileVersion = 4;
    toStream << v4FileVersion;
    int numBytes = 0;
    char buf[256];
    do {
        numBytes = fromStream.readRawData(buf, 256);
        toStream.writeRawData(buf, numBytes);
    } while (numBytes != 0);

    return true;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
