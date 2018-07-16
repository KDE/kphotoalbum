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
#include "Logging.h"
#include "OptimizedFileList.h"

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
}

#include <QCryptographicHash>
#include <QFile>
#include <QLoggingCategory>

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
    for ( const QString &fileName : m_fileList ) {
        QString dir = getDirName(fileName);
        if (! dirMap.contains( dir ) ) {
            StringSet newDir;
            dirMap.insert(dir, newDir);
            dirList << dir;
        }
        dirMap[ dir ] << fileName;
    }
    struct stat statbuf;
    for ( const QString &dirName : dirList ) {
        const StringSet &files(dirMap[dirName]);
        FastDir dir(dirName);
        QStringList sortedList = dir.sortFileList(files);
        QString fsName( QString::fromLatin1( "NULLFS" ) );
        if ( stat( QByteArray(QFile::encodeName(dirName)).constData(), &statbuf ) == 0 ) {
            QCryptographicHash md5calculator(QCryptographicHash::Md5);
            QByteArray md5Buffer((const char *) &(statbuf.st_dev), sizeof(statbuf.st_dev));
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
        for (const QString &fs : filesystemsToRemove) {
            tmpFsMap.remove( fs );
        }
    }
    if ( tmpFsMap.size() > 0 ) {
        QStringList &remainder(tmpFsMap.last());
        m_optimizedList += remainder;
    }
    //    for (QStringList::iterator it = m_optimizedList.begin(); it != m_optimizedList.end(); ++it) {
    //        qDebug() << *it;
    //    }
    m_haveOptimizedFiles = true;
}

QStringList DB::OptimizedFileList::optimizedFiles() const
{
    return m_optimizedList;
}

DB::FileNameList DB::OptimizedFileList::optimizedDbFiles() const
{
    return FileNameList(m_optimizedList);
}

// vi:expandtab:tabstop=4 shiftwidth=4:
