/* SPDX-FileCopyrightText: 2003-2019 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "FileUtil.h"

#include <QDir>
#include <QFileInfo>

extern "C" {
#include <unistd.h>
}

bool Utilities::copyOrOverwrite(const QString &from, const QString &to)
{
    if (QFileInfo(to).exists())
        QDir().remove(to);
    return QFile::copy(from, to);
}

bool Utilities::makeHardLink(const QString &from, const QString &to)
{
    if (link(from.toLocal8Bit().constData(), to.toLocal8Bit().constData()) != 0)
        return false;
    else
        return true;
}

bool Utilities::makeSymbolicLink(const QString &from, const QString &to)
{
    if (symlink(from.toLocal8Bit().constData(), to.toLocal8Bit().constData()) != 0)
        return false;
    else
        return true;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
