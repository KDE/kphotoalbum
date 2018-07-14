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

#include "FastDir.h"
#include "OptimizedFileList.h"

extern "C" {
#include <sys/types.h>
#include <dirent.h>

#ifdef __linux__
# include <sys/vfs.h>
#else
#ifdef __FreeBSD__
# include <sys/param.h>
# include <sys/mount.h>
#else
#warning "Unsupported platform. Please file a bug report or send a patch!"
#endif
#endif
}

#include <QFile>
#include <QCryptographicHash>

DB::OptimizedFileList::OptimizedFileList(const QStringList &files)
    : m_fileList(files),
      m_haveOptimizedFiles(false)
{
    optimizeFiles();
}

DB::OptimizedFileList::OptimizedFileList(const DB::FileNameList &files)
    : m_fileList(files.toStringList(DB::AbsolutePath)),
      m_haveOptimizedFiles(false)
{
    optimizeFiles();
}

QString DB::OptimizedFileList::getDirName(const QString& path)
{
    static const QString pathSep(QString::fromLatin1("/"));
    int lastChar = path.lastIndexOf(pathSep);
    if (lastChar <= 0)
        return QString::fromLatin1("./");
    else
        return path.left(lastChar + 1);
}

void DB::OptimizedFileList::optimizeFiles() const
{
    if ( m_haveOptimizedFiles )
        return;
    DirMap dirMap;
    QStringList dirList;
    // Map files to directories
    for ( const QString fileName : m_fileList ) {
        QString dir = getDirName(fileName);
        if (! dirMap.contains( dir ) ) {
            StringSet newDir;
            dirMap.insert(dir, newDir);
            dirList << dir;
        }
        dirMap[ dir ] << fileName;
    }
    struct statfs statbuf;
    for ( QString dirName : dirList ) {
        const StringSet &files(dirMap[dirName]);
        FastDir dir(dirName);
        QStringList sortedList = dir.sortFileList(files);
        QString fsName( QString::fromLatin1( "NULLFS" ) );
        if ( statfs( QByteArray(QFile::encodeName(dirName)).constData(), &statbuf ) == 0 ) {
            QCryptographicHash md5calculator(QCryptographicHash::Md5);
            QByteArray md5Buffer((const char *) &(statbuf.f_fsid), sizeof(statbuf.f_fsid));
            md5calculator.addData(md5Buffer);
            fsName = QString::fromLatin1(md5calculator.result().toHex());
        }
        if ( ! m_fsMap.contains( fsName ) ) {
            QStringList newList;
            m_fsMap.insert(fsName, newList);
        }
        m_fsMap[ fsName ] += sortedList;
    }
    FSMap tmpFsMap(m_fsMap);
    while ( tmpFsMap.size() > 1 ) {
        QStringList filesystemsToRemove;
        for (FSMap::iterator it = tmpFsMap.begin(); it != tmpFsMap.end(); ++it) {
            if ( it.value().length() > 0 ) {
                m_optimizedList.append(it.value().takeFirst());
            } else {
                filesystemsToRemove << it.key();
            }
        }
        for (QString fs : filesystemsToRemove) {
            tmpFsMap.remove( fs );
        }
    }
    if ( tmpFsMap.size() > 0 ) {
        QStringList &remainder(tmpFsMap.last());
        m_optimizedList += remainder;
    }
    m_haveOptimizedFiles = true;
}

const QStringList DB::OptimizedFileList::optimizedFiles() const
{
    return m_optimizedList;
}

DB::FileNameList DB::OptimizedFileList::dbListFromStrings(const QStringList &files)
{
    DB::FileNameList answer;
    for (QString fileName : files)
        answer << DB::FileName::fromAbsolutePath(fileName);
    return answer;
}

const DB::FileNameList DB::OptimizedFileList::optimizedDbFiles() const
{
    return dbListFromStrings(m_optimizedList);
}

const QStringList DB::OptimizedFileList::getFilesystemIDs() const
{
    return m_fsMap.keys();
}

const QStringList DB::OptimizedFileList::getFilesByFilesystem(const QString &id) const
{
    if ( m_fsMap.contains( id ) )
        return m_fsMap[id];
    else
        return QStringList();
}

const DB::FileNameList DB::OptimizedFileList::getDbFilesByFilesystem(const QString &id) const
{
    if ( m_fsMap.contains( id ) )
        return dbListFromStrings(m_fsMap[id]);
    else
        return DB::FileNameList();
}

// vi:expandtab:tabstop=4 shiftwidth=4:
