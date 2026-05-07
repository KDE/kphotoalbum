// SPDX-FileCopyrightText: 2012 - 2025 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2024 Tobias Leupold <tl@stonemx.de>
// SPDX-FileCopyrightText: 2026 Randall Rude <rsquared42@proton.me>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "DuplicatesFinder.h"
#include <DB/ImageDB.h>

#include <QFileInfo>

bool DB::DuplicatesFinder::findDuplicates()
{
    bool foundDuplicate = false;

    const auto images = DB::ImageDB::instance()->files();
    for (const DB::FileName &fileName : images) {
        const DB::ImageInfoPtr info = DB::ImageDB::instance()->info(fileName);
        const DB::MD5 md5 = info->MD5Sum();

        if (!foundDuplicate && m_duplicates.contains(md5)) {
            foundDuplicate = true;
        }

        m_duplicates[md5].append(fileName);
    }

    if (!foundDuplicate) {
        return false;
    }

    // Sort any duplicates in order of increasing birth time to make the
    // positioning in the table consistent (ie. the oldest duplicate in each
    // row is in the first column and the newest duplicate is in the last
    // column).
    for (QMap<DB::MD5, DB::FileNameList>::iterator it = m_duplicates.begin();
         it != m_duplicates.end(); ++it) {
        if (it.value().count() > 1) {
            std::sort(it.value().begin(), it.value().end(),
                      [](DB::FileName a, DB::FileName b) {
                          const QFileInfo aInfo(a.absolute());
                          const QFileInfo bInfo(b.absolute());

                          return aInfo.birthTime() < bInfo.birthTime();
                      });
        }
    }

    return true;
}
// vi:expandtab:tabstop=4 shiftwidth=4:
