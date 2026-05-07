// SPDX-FileCopyrightText: 2012 - 2022 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2012 - 2025 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2026 Randall Rude <rsquared42@proton.me>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DUPLICATESFINDER_H
#define DUPLICATESFINDER_H

#include <DB/MD5.h>
#include <kpabase/FileNameList.h>

#include <QMap>

namespace DB
{
typedef QMap<DB::MD5, DB::FileNameList> DuplicatesType;

/**
 * Searches the XML database for duplicates (files with the same MD5 sum).
 */
class DuplicatesFinder
{
public:
    /**
     * Finds any duplicate files in the XML database.
     *
     * @returns true if at least one duplicate file is found.
     */
    bool findDuplicates();

    const DB::DuplicatesType& getDuplicates()
    {
        return m_duplicates;
    }

private:

    DB::DuplicatesType m_duplicates;
};
}

#endif /* DUPLICATESFINDER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
