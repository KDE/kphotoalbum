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
#include "Logging.h"

#include <QFile>
#include <QDataStream>
#include <QMutexLocker>
#include <QThread>
#include <QAtomicInt>

extern "C" {
#include <fcntl.h>
#include <unistd.h>
}

using namespace DB;

namespace {
constexpr int DEFAULT_SCOUT_BUFFER_SIZE = 1048576; // *sizeof(int) bytes
// We might want this to be bytes rather than images.
constexpr int DEFAULT_MAX_SEEKAHEAD_IMAGES = 10;
constexpr int SEEKAHEAD_WAIT_MS = 10; // 10 milliseconds, and retry
constexpr int TERMINATION_WAIT_MS = 10; // 10 milliseconds, and retry
}

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
                      QAtomicInt &preloadCount, QAtomicInt &skippedCount );
protected:
    virtual void run();
    void setBufSize(int);
    int getBufSize();
    void setMaxSeekAhead(int);
    int getMaxSeekAhead();
    void setReadLimit(int);
    int getReadLimit();
private:
    void doRun(char *);
    ImageScoutQueue& m_queue;
    QMutex *m_mutex;
    QAtomicInt& m_loadedCount;
    QAtomicInt& m_preloadedCount;
    QAtomicInt& m_skippedCount;
    int m_scoutBufSize;
    int m_maxSeekAhead;
    int m_readLimit;
    bool m_isStarted;
};

ImageScoutThread::ImageScoutThread( ImageScoutQueue &queue, QMutex *mutex,
                                    QAtomicInt &count, 
                                    QAtomicInt &preloadedCount, 
                                    QAtomicInt &skippedCount )
  : m_queue(queue),
    m_mutex(mutex),
    m_loadedCount(count),
    m_preloadedCount(preloadedCount),
    m_skippedCount(skippedCount),
    m_scoutBufSize(DEFAULT_SCOUT_BUFFER_SIZE),
    m_maxSeekAhead(DEFAULT_MAX_SEEKAHEAD_IMAGES),
    m_readLimit(-1),
    m_isStarted(false)
{
}

void ImageScoutThread::doRun(char *tmpBuf)
{
    while ( !isInterruptionRequested() ) {
        QMutexLocker locker(m_mutex);
        if ( m_queue.isEmpty() ) {
            return;
        }
        DB::FileName fileName = m_queue.dequeue();
        locker.unlock();
        // If we're behind the reader, move along
        m_preloadedCount++;
        if ( m_loadedCount.load() >= m_preloadedCount.load() ) {
            m_skippedCount++;
            continue;
        } else {
            // Don't get too far ahead of the loader, or we just waste memory
            // TODO: wait on something rather than polling
            while (m_preloadedCount.load() >= m_loadedCount.load() + m_maxSeekAhead &&
                   ! isInterruptionRequested()) {
                QThread::msleep(SEEKAHEAD_WAIT_MS);
            }
            // qCDebug(DBImageScoutLog) << ">>>>>Scout: preload" << m_preloadedCount.load() << "load" << m_loadedCount.load() << fileName.relative();
        }
        int inputFD = open( QFile::encodeName( fileName.absolute()).constData(), O_RDONLY );
        int bytesRead = 0;
        if ( inputFD >= 0 ) {
            while ( read( inputFD, tmpBuf, m_scoutBufSize ) &&
                    ( m_readLimit < 0 ||
                      ( (bytesRead += m_scoutBufSize) < m_readLimit ) ) &&
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

void ImageScoutThread::setReadLimit(int readLimit)
{
    if ( ! m_isStarted )
        m_readLimit = readLimit;
}

int ImageScoutThread::getReadLimit()
{
    return m_readLimit;
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
    : m_preloadedCount(0),
      m_skippedCount(0),
      m_isStarted(false),
      m_scoutBufSize(DEFAULT_SCOUT_BUFFER_SIZE),
      m_maxSeekAhead(DEFAULT_MAX_SEEKAHEAD_IMAGES),
      m_readLimit(-1)
{
    if (threads > 0) {
        for (int i = 0; i < threads; i++) {
            ImageScoutThread *t =  
                new ImageScoutThread( images,
                                      threads > 1 ? &m_mutex : nullptr,
                                      count,
                                      m_preloadedCount,
                                      m_skippedCount );
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
                        QThread::msleep(TERMINATION_WAIT_MS);
                }
            }
            delete (*it);
        }
    }
    qCDebug(DBImageScoutLog) << "Total files:" << m_preloadedCount << "skipped" << m_skippedCount;
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

void ImageScout::setReadLimit(int readLimit)
{
    if ( ! m_isStarted && readLimit > 0 ) {
        m_readLimit = readLimit;
        for ( QList<ImageScoutThread *>::iterator it = m_scoutList.begin();
              it != m_scoutList.end(); ++it ) {
            (*it)->setReadLimit( m_readLimit );
        }
    }
}

int ImageScout::getReadLimit()
{
    return m_readLimit;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
