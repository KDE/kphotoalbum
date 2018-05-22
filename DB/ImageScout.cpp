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
#include <QThread>

extern "C" {
#include <fcntl.h>
#include <unistd.h>
}

using namespace DB;

#define M_LOCK(l) do { if ( l ) (l)->lock(); } while (0)
#define M_UNLOCK(l) do { if ( l ) (l)->unlock(); } while (0)

static const int defaultScoutBufSize = 1048576;
// We might want this to be bytes rather than images.
static const int defaultMaxSeekAhead = 10;
static const int seekAheadWait = 10; // 10 milliseconds, and retry
static const int terminationWait = 10; // 10 milliseconds, and retry

// 1048576 with a single scout thread empirically yields best performance
// on a Seagate 2TB 2.5" disk, sustaining throughput in the range of
// 95-100 MB/sec with 100-110 IO/sec on large files.  This is close to what
// would be expected.  A SATA SSD (Crucial MX300) is much less sensitive to
// I/O size and scout thread, achieving about 340 MB/sec with high CPU
// utilization.

class DB::ImageScoutThread :public QThread {
    friend class DB::ImageScout;
public:
    ImageScoutThread( ImageScoutQueue &, QMutex *, QAtomicInt &count,
                      QAtomicInt &preloadCount, int index);
protected:
    virtual void run();
    void setBufSize(int);
    int getBufSize();
    void setMaxSeekAhead(int);
    int getMaxSeekAhead();
private:
    void doRun(char *);
    ImageScoutQueue& m_queue;
    QMutex *m_mutex;
    QAtomicInt& m_loadedCount;
    QAtomicInt& m_preloadedCount;
    int m_scoutBufSize;
    int m_maxSeekAhead;
    int m_index;
    bool m_isStarted;
};

ImageScoutThread::ImageScoutThread( ImageScoutQueue &queue, QMutex *mutex,
                                    QAtomicInt &count, 
                                    QAtomicInt &preloadedCount, int index)
  : m_queue(queue),
    m_mutex(mutex),
    m_loadedCount(count),
    m_preloadedCount(preloadedCount),
    m_scoutBufSize(defaultScoutBufSize),
    m_maxSeekAhead(defaultMaxSeekAhead),
    m_index(index),
    m_isStarted(false)
{
}

void ImageScoutThread::doRun(char *tmpBuf)
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
            // TODO: wait on something rather than polling
            while (m_preloadedCount.load() >= m_loadedCount.load() + m_maxSeekAhead &&
                   ! isInterruptionRequested()) {
                QThread::msleep(seekAheadWait);
            }
        }
        int inputFD = open( QFile::encodeName( fileName.absolute()).constData(), O_RDONLY );
        if ( inputFD >= 0 ) {
            while ( read( inputFD, tmpBuf, m_scoutBufSize ) &&
                    ! isInterruptionRequested() ) {
            }
            (void) close( inputFD );
        }
    }
}

void ImageScoutThread::setBufSize(int bufSize)
{
    if ( ! m_isStarted )
        m_scoutBufSize = bufSize;
}

int ImageScoutThread::getBufSize()
{
    return m_scoutBufSize;
}

void ImageScoutThread::setMaxSeekAhead(int maxSeekAhead)
{
    if ( ! m_isStarted )
        m_maxSeekAhead = maxSeekAhead;
}

int ImageScoutThread::getMaxSeekAhead()
{
    return m_maxSeekAhead;
}

void ImageScoutThread::run()
{
    m_isStarted = true;
    char *tmpBuf = new char[m_scoutBufSize];
    doRun( tmpBuf );
    delete[] tmpBuf;
}

ImageScout::ImageScout(ImageScoutQueue &images,
                       QAtomicInt &count,
                       int threads)
{
    if (threads > 0) {
        for (int i = 0; i < threads; i++) {
            ImageScoutThread *t =  
                new ImageScoutThread( images,
                                      threads > 1 ? &m_mutex : nullptr,
                                      count,
                                      m_preloadedCount,
                                      i);
            m_scoutList.append( t );
        }
    }
}

ImageScout::~ImageScout()
{
    if ( m_scoutList.count() > 0 ) {
        for ( QList<ImageScoutThread *>::iterator it = m_scoutList.begin();
              it != m_scoutList.end(); ++it ) {
            if (m_isStarted) {
                if ( ! (*it)->isFinished() ) {
                    (*it)->requestInterruption();
                    while ( ! (*it)->isFinished() )
                        QThread::msleep(terminationWait);
                }
            }
            delete (*it);
        }
    }
}

void ImageScout::start()
{
    // Yes, there's a race condition here between isStartd and setting
    // the buf size or seek ahead...but this isn't a hot code path!
    if ( ! m_isStarted && m_scoutList.count() > 0 ) {
        m_isStarted = true;
        for ( QList<ImageScoutThread *>::iterator it = m_scoutList.begin();
              it != m_scoutList.end(); ++it ) {
            (*it)->start();
        }
    }
}

void ImageScout::setBufSize(int bufSize)
{
    if ( ! m_isStarted && bufSize > 0 ) {
        m_scoutBufSize = bufSize;
        for ( QList<ImageScoutThread *>::iterator it = m_scoutList.begin();
              it != m_scoutList.end(); ++it ) {
            (*it)->setBufSize( m_scoutBufSize );
        }
    }
}

int ImageScout::getBufSize()
{
    return m_scoutBufSize;
}

void ImageScout::setMaxSeekAhead(int maxSeekAhead)
{
    if ( ! m_isStarted && maxSeekAhead > 0 ) {
        m_maxSeekAhead = maxSeekAhead;
        for ( QList<ImageScoutThread *>::iterator it = m_scoutList.begin();
              it != m_scoutList.end(); ++it ) {
            (*it)->setMaxSeekAhead( m_maxSeekAhead );
        }
    }
}

int ImageScout::getMaxSeekAhead()
{
    return m_maxSeekAhead;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
