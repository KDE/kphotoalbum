/* SPDX-FileCopyrightText: 2018 Robert Krawitz <rlk@alum.mit.edu>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef OPTIMIZEDFILELIST_H
#define OPTIMIZEDFILELIST_H
#include "FileNameList.h"

#include <QMap>
#include <QSet>
#include <QString>
#include <QStringList>

namespace DB
{
typedef QSet<QString> StringSet;
typedef QMap<QString, StringSet> DirMap;
// Key is MD5 hash of the (opaque) contents of f_fsid
typedef QMap<QString, QStringList> FSMap;

/**
 * Provide a list of files optimized by filesystem.
 * File names are interleaved across all filesystems
 * with files belonging to them.
 *
 * In other words, you can put in a list of files, and get
 * back a list that is optimized for read performance.
 */
class OptimizedFileList
{
public:
    explicit OptimizedFileList(const DB::FileNameList &files);
    explicit OptimizedFileList(const QStringList &files);
    QStringList optimizedFiles() const;
    DB::FileNameList optimizedDbFiles() const;

private:
    OptimizedFileList();
    void optimizeFiles() const;
    const QStringList m_fileList;
    mutable QStringList m_optimizedList;
    mutable bool m_haveOptimizedFiles;
    mutable FSMap m_fsMap;
    static QString getDirName(const QString &);
};

}

#endif /* OPTIMIZEDFILELIST_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
