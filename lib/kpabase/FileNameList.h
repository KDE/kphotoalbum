/* SPDX-FileCopyrightText: 2012-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KPABASE_FILENAMELIST_H
#define KPABASE_FILENAMELIST_H

#include "FileName.h"

#include <QList>
#include <QStringList>

namespace DB
{
enum PathType {
    RelativeToImageRoot,
    AbsolutePath
};

class FileNameList : public QList<DB::FileName>
{
public:
    FileNameList() { }
    FileNameList(const QList<DB::FileName> &);
    /**
     * @brief Create a FileNameList from a list of absolute filenames.
     * @param files
     */
    explicit FileNameList(const QStringList &files);
    QStringList toStringList(DB::PathType) const;
    FileNameList &operator<<(const DB::FileName &);
    FileNameList reversed() const;
};

}

#endif // KPABASE_FILENAMELIST_H
// vi:expandtab:tabstop=4 shiftwidth=4:
