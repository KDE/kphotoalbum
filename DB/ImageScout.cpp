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

#include "ImageScout.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>

extern "C" {
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
}

using namespace DB;

#define M_LOCK(l) do { if ( l ) (l)->lock(); } while (0)
#define M_UNLOCK(l) do { if ( l ) (l)->unlock(); } while (0)

// 1048576 with a single scout thread empirically yields best performance
// on a Seagate 2TB 2.5" disk, sustaining throughput in the range of
// 95-100 MB/sec with 100-110 IO/sec on large files.  This is close to what
// would be expected.  A SATA SSD (Crucial MX300) is much less sensitive to
// I/O size and scout thread, achieving about 340 MB/sec with high CPU
// utilization.
const int scoutBufSize = 1048576;

ImageScoutThread::ImageScoutThread( ImageScoutQueue &queue, QMutex *mutex,
                                    QAtomicInt &count,
                                    QAtomicInt &preloadedCount, int index)
  : m_queue(queue),
    m_mutex(mutex),
    m_loadedCount(count),
    m_preloadedCount(preloadedCount),
    m_tmpBuf( new char[scoutBufSize] ),
    m_index(index)
{
}

ImageScoutThread::~ImageScoutThread()
{
    delete[] m_tmpBuf;
}

void ImageScoutThread::run()
{
    while ( !isInterruptionRequested() ) {
        M_LOCK(m_mutex);
        if ( m_queue.isEmpty() ) {
	    M_UNLOCK(m_mutex);
	    return;
        }
        DB::FileName fileName = m_queue.dequeue();
	M_UNLOCK(m_mutex);
        // If we're behind the reader, move along
        m_preloadedCount++;
        if ( m_loadedCount.load() >= m_preloadedCount.load() ) {
            continue;
        } else {
            // Don't get too far ahead of the loader, or we just waste memory
            while (m_preloadedCount.load() >= m_loadedCount.load() + 10 &&
                   ! isInterruptionRequested()) {
                QThread::msleep(10);
            }
        }
        int inputFD = open( QFile::encodeName( fileName.absolute()).constData(), O_RDONLY );
        if ( inputFD >= 0 ) {
            while ( read( inputFD, m_tmpBuf, scoutBufSize ) &&
                    ! isInterruptionRequested() ) {
            }
            (void) close( inputFD );
        }
    }
}
// vi:expandtab:tabstop=4 shiftwidth=4:
