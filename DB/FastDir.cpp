/* Copyright (C) 2010-2018 Jesper Pedersen <blackie@blackie.dk> and
   Robert Krawitz <rlk@alum.mit.edu>

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

#include "FastDir.h"
#include "Logging.h"

#include <QFile>
#include <QLoggingCategory>
#include <QMap>

extern "C" {
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>

/*
 * Ideally the order of entries returned by readdir() should be close
 * to optimal; intuitively, it should reflect the order in which inodes
 * are returned from getdents() or equivalent, which should be the order
 * in which they're stored on the disk.  Experimentally, that isn't always
 * true.  One test, involving 10839 files totaling 90 GB resulted in
 * readdir() returning files in random order, where "find" returned them
 * sorted (and the same version of find does *not* in general return files
 * in alphabetical order).
 *
 * By repeated measurement, loading the files in the order returned by
 * readdir took about 16:30, where loading them in alphanumeric sorted order
 * took about 15:00.  Running a similar test outside of kpa (using the order
 * returned by readdir() vs. sorted to cat the files through dd and measuring
 * the time) yielded if anything an even greater discrepancy (17:35 vs. 14:10).
 *
 * This issue is filesystem dependent, but is known to affect the extN
 * filesystems commonly used on Linux that use a hashed tree structure to
 * store directories.  See e. g.
 * http://home.ifi.uio.no/paalh/publications/files/ipccc09.pdf and its
 * accompanying presentation
 * http://www.linux-kongress.org/2009/slides/linux_disk_io_performance_havard_espeland.pdf
 *
 * We could do even better by sorting by block position, but that would
 * greatly increase complexity.
 */
#ifdef __linux__
#  include <sys/vfs.h>
#  include <linux/magic.h>
#  define HAVE_STATFS
#  define STATFS_FSTYPE_EXT2 EXT2_SUPER_MAGIC  // Includes EXT3_SUPER_MAGIC, EXT4_SUPER_MAGIC
#else
#ifdef __FreeBSD__
#  include <sys/param.h>
#  include <sys/mount.h>
#  include <sys/disklabel.h>
#  define HAVE_STATFS
#  define STATFS_FSTYPE_EXT2 FS_EXT2FS
#endif
// other platforms fall back to known-safe (but slower) implementation
#endif  // __linux__
}

typedef QMap<ino_t, QString> InodeMap;

typedef QSet<QString> StringSet;

DB::FastDir::FastDir(const QString &path)
  : m_path(path)
{
    InodeMap tmpAnswer;
    DIR *dir;
    dirent *file;
    QByteArray bPath(QFile::encodeName(path));
    dir = opendir( bPath.constData() );
    if ( !dir )
        return;
    const bool doSortByInode = sortByInode(bPath);
    const bool doSortByName = sortByName(bPath);

#if defined(QT_THREAD_SUPPORT) && defined(_POSIX_THREAD_SAFE_FUNCTIONS) && !defined(Q_OS_CYGWIN)
    // ZaJ (2016-03-23): while porting to Qt5/KF5, this code-path is disabled on my system
    //     I don't want to touch this right now since I can't verify correctness in any way.
    // rlk 2018-05-20: readdir_r is deprecated as of glibc 2.24; see
    //     http://man7.org/linux/man-pages/man3/readdir_r.3.html.
    //     There are problems with MAXNAMLEN/NAME_MAX and friends, that
    //     can differ from filesystem to filesystem.  It's also expected
    //     that POSIX will (if it hasn't already) deprecate readdir_r
    //     and require readdir to be thread safe.
    union dirent_buf {
        struct KDE_struct_dirent mt_file;
        char b[sizeof(struct dirent) + MAXNAMLEN + 1];
    } *u = new union dirent_buf;
    while ( readdir_r(dir, &(u->mt_file), &file ) == 0 && file )
#else
    // FIXME: use 64bit versions of readdir and dirent?
    while ( (file = readdir(dir)) )
#endif // QT_THREAD_SUPPORT && _POSIX_THREAD_SAFE_FUNCTIONS
    {
        if ( doSortByInode )
            tmpAnswer.insert(file->d_ino, QFile::decodeName(file->d_name));
        else
            m_sortedList.append(QFile::decodeName(file->d_name));
    }
#if defined(QT_THREAD_SUPPORT) && defined(_POSIX_THREAD_SAFE_FUNCTIONS) && !defined(Q_OS_CYGWIN)
    delete u;
#endif
    (void) closedir(dir);
    
    if ( doSortByInode ) {
        for ( InodeMap::iterator it = tmpAnswer.begin(); it != tmpAnswer.end(); ++it ) {
            m_sortedList << it.value();
        }
    } else if ( doSortByName ) {
        m_sortedList.sort();
    }
}

// No currently known filesystems where sort by name is optimal
constexpr bool DB::sortByName(const QByteArray &)
{
    return false;
}

bool DB::sortByInode(const QByteArray &path)
{
#ifdef HAVE_STATFS
    struct statfs buf;
    if ( statfs( path.constData(), &buf ) == -1 )
        return -1;
    // Add other filesystems as appropriate
    switch ( buf.f_type ) {
        case STATFS_FSTYPE_EXT2:
            return true;
        default:
            return false;
    }
#else   // HAVE_STATFS
    Q_UNUSED(path);
    return false;
#endif  // HAVE_STATFS
}

const QStringList DB::FastDir::entryList() const
{
    return m_sortedList;
}

QStringList DB::FastDir::sortFileList(const StringSet &files) const
{
    QStringList answer;
    StringSet tmp(files);
    for ( const QString &fileName : m_sortedList ) {
        if ( tmp.contains( fileName ) ) {
            answer << fileName;
            tmp.remove( fileName );
        } else if ( tmp.contains( m_path + fileName ) ) {
            answer << m_path + fileName;
            tmp.remove( m_path + fileName );
        }
    }
    if ( tmp.count() > 0 ) {
        qCDebug(FastDirLog) << "Files left over after sorting on " << m_path;
        for ( const QString &fileName : tmp ) {
            qCDebug(FastDirLog) << fileName;
            answer << fileName;
        }
    }
    return answer;
}

QStringList DB::FastDir::sortFileList(const QStringList &files) const
{
    StringSet tmp;
    for ( const QString &fileName : files ) {
        tmp << fileName;
    }
    return sortFileList(tmp);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
