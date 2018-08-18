/* Copyright (C) 2018 Robert Krawitz <rlk@alum.mit.edu>

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

#ifndef OPTIMIZEDFILELIST_H
#define OPTIMIZEDFILELIST_H
#include <QStringList>
#include <QString>
#include <QSet>
#include <DB/FileNameList.h>

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
