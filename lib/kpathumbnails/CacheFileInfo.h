// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef KPATHUMBNAILS_CACHEFILEINFO_H
#define KPATHUMBNAILS_CACHEFILEINFO_H

namespace ImageManager
{
struct CacheFileInfo {
    CacheFileInfo() { }
    CacheFileInfo(int fileIndex, int offset, int size)
        : fileIndex(fileIndex)
        , offset(offset)
        , size(size)
    {
    }

    int fileIndex = 0;
    int offset = 0;
    int size = 0;
};
}

#endif /* KPATHUMBNAILS_CACHEFILEINFO_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
