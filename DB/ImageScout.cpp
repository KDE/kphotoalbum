// SPDX-FileCopyrightText: 2018 - 2020 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2022 - 2024 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ImageScout.h"

#include <kpabase/Logging.h>

#include <QAtomicInt>
#include <QDataStream>
#include <QFile>
#include <QMutexLocker>
#include <QThread>

extern "C" {
#include <fcntl.h>
#include <unistd.h>
}

using namespace DB;

namespace
{
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

class DB::ImageScoutThread : public QThread
{
    friend class DB::ImageScout;

public:
    ImageScoutThread(ImageScoutQueue &, QMutex *, QAtomicInt &count,
                     QAtomicInt &preloadCount, QAtomicInt &skippedCount);

protected:
    void run() override;
    void setBufSize(int);
    int getBufSize();
    void setMaxSeekAhead(int);
    int getMaxSeekAhead();
    void setReadLimit(int);
    int getReadLimit();
    void setPreloadFunc(PreloadFunc);
    PreloadFunc getPreloadFunc();

private:
    void doRun(char *);
    ImageScoutQueue &m_shared_queue;
    QMutex *m_shared_mutex;
    QAtomicInt &m_shared_loadedCount;
    QAtomicInt &m_shared_preloadedCount;
    QAtomicInt &m_shared_skippedCount;
    int m_scoutBufSize;
    int m_maxSeekAhead;
    int m_readLimit;
    PreloadFunc m_preloadFunc;
    bool m_isStarted;
};

ImageScoutThread::ImageScoutThread(ImageScoutQueue &queue, QMutex *mutex,
                                   QAtomicInt &count,
                                   QAtomicInt &preloadedCount,
                                   QAtomicInt &skippedCount)
    : m_shared_queue(queue)
    , m_shared_mutex(mutex)
    , m_shared_loadedCount(count)
    , m_shared_preloadedCount(preloadedCount)
    , m_shared_skippedCount(skippedCount)
    , m_scoutBufSize(DEFAULT_SCOUT_BUFFER_SIZE)
    , m_maxSeekAhead(DEFAULT_MAX_SEEKAHEAD_IMAGES)
    , m_readLimit(-1)
    , m_preloadFunc(nullptr)
    , m_isStarted(false)
{
    Q_ASSERT(m_shared_mutex);
}

void ImageScoutThread::doRun(char *tmpBuf)
{
    while (!isInterruptionRequested()) {
        QMutexLocker locker(m_shared_mutex);
        if (m_shared_queue.isEmpty()) {
            return;
        }
        DB::FileName fileName = m_shared_queue.dequeue();
        locker.unlock();
        // If we're behind the reader, move along
        m_shared_preloadedCount++;
        if (m_shared_loadedCount.loadRelaxed() >= m_shared_preloadedCount.loadRelaxed()) {
            m_shared_skippedCount++;
            continue;
        } else {
            // Don't get too far ahead of the loader, or we just waste memory
            // TODO: wait on something rather than polling
            while (m_shared_preloadedCount.loadRelaxed() >= m_shared_loadedCount.loadRelaxed() + m_maxSeekAhead && !isInterruptionRequested()) {
                QThread::msleep(SEEKAHEAD_WAIT_MS);
            }
            // qCDebug(DBImageScoutLog) << ">>>>>Scout: preload" << m_preloadedCount.loadRelaxed() << "load" << m_loadedCount.loadRelaxed() << fileName.relative();
        }
        if (m_preloadFunc) {
            (*m_preloadFunc)(fileName);
        } else {
            // Note(jzarl): for Windows, we'd need a functional replacement for open(), read(), close() in unistd.h
            int inputFD = open(QFile::encodeName(fileName.absolute()).constData(), O_RDONLY);
            int bytesRead = 0;
            if (inputFD >= 0) {
                while (read(inputFD, tmpBuf, m_scoutBufSize) && (m_readLimit < 0 || ((bytesRead += m_scoutBufSize) < m_readLimit)) && !isInterruptionRequested()) {
                }
                (void)close(inputFD);
            }
        }
    }
}

void ImageScoutThread::setBufSize(int bufSize)
{
    if (!m_isStarted)
        m_scoutBufSize = bufSize;
}

int ImageScoutThread::getBufSize()
{
    return m_scoutBufSize;
}

void ImageScoutThread::setMaxSeekAhead(int maxSeekAhead)
{
    if (!m_isStarted)
        m_maxSeekAhead = maxSeekAhead;
}

int ImageScoutThread::getMaxSeekAhead()
{
    return m_maxSeekAhead;
}

void ImageScoutThread::setReadLimit(int readLimit)
{
    if (!m_isStarted)
        m_readLimit = readLimit;
}

int ImageScoutThread::getReadLimit()
{
    return m_readLimit;
}

void ImageScoutThread::setPreloadFunc(PreloadFunc scoutFunc)
{
    if (!m_isStarted)
        m_preloadFunc = scoutFunc;
}

PreloadFunc ImageScoutThread::getPreloadFunc()
{
    return m_preloadFunc;
}

void ImageScoutThread::run()
{
    m_isStarted = true;
    char *tmpBuf = new char[m_scoutBufSize];
    doRun(tmpBuf);
    delete[] tmpBuf;
}

ImageScout::ImageScout(ImageScoutQueue &images,
                       QAtomicInt &count,
                       int threads)
    : m_preloadedCount(0)
    , m_skippedCount(0)
    , m_isStarted(false)
    , m_scoutBufSize(DEFAULT_SCOUT_BUFFER_SIZE)
    , m_maxSeekAhead(DEFAULT_MAX_SEEKAHEAD_IMAGES)
    , m_readLimit(-1)
    , m_preloadFunc(nullptr)
{
    if (threads > 0) {
        for (int i = 0; i < threads; i++) {
            ImageScoutThread *t = new ImageScoutThread(images,
                                                       &m_mutex,
                                                       count,
                                                       m_preloadedCount,
                                                       m_skippedCount);
            m_scoutList.append(t);
        }
    }
}

ImageScout::~ImageScout()
{
    if (m_scoutList.count() > 0) {
        for (QList<ImageScoutThread *>::iterator it = m_scoutList.begin();
             it != m_scoutList.end(); ++it) {
            if (m_isStarted) {
                if (!(*it)->isFinished()) {
                    (*it)->requestInterruption();
                    while (!(*it)->isFinished())
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
    if (!m_isStarted && m_scoutList.count() > 0) {
        m_isStarted = true;
        for (QList<ImageScoutThread *>::iterator it = m_scoutList.begin();
             it != m_scoutList.end(); ++it) {
            (*it)->start();
        }
    }
}

void ImageScout::setBufSize(int bufSize)
{
    if (!m_isStarted && bufSize > 0) {
        m_scoutBufSize = bufSize;
        for (QList<ImageScoutThread *>::iterator it = m_scoutList.begin();
             it != m_scoutList.end(); ++it) {
            (*it)->setBufSize(m_scoutBufSize);
        }
    }
}

int ImageScout::getBufSize()
{
    return m_scoutBufSize;
}

void ImageScout::setMaxSeekAhead(int maxSeekAhead)
{
    if (!m_isStarted && maxSeekAhead > 0) {
        m_maxSeekAhead = maxSeekAhead;
        for (QList<ImageScoutThread *>::iterator it = m_scoutList.begin();
             it != m_scoutList.end(); ++it) {
            (*it)->setMaxSeekAhead(m_maxSeekAhead);
        }
    }
}

int ImageScout::getMaxSeekAhead()
{
    return m_maxSeekAhead;
}

void ImageScout::setReadLimit(int readLimit)
{
    if (!m_isStarted && readLimit > 0) {
        m_readLimit = readLimit;
        for (QList<ImageScoutThread *>::iterator it = m_scoutList.begin();
             it != m_scoutList.end(); ++it) {
            (*it)->setReadLimit(m_readLimit);
        }
    }
}

int ImageScout::getReadLimit()
{
    return m_readLimit;
}

void ImageScout::setPreloadFunc(PreloadFunc scoutFunc)
{
    if (!m_isStarted) {
        m_preloadFunc = scoutFunc;
        for (QList<ImageScoutThread *>::iterator it = m_scoutList.begin();
             it != m_scoutList.end(); ++it) {
            (*it)->setPreloadFunc(m_preloadFunc);
        }
    }
}

PreloadFunc ImageScout::getPreloadFunc()
{
    return m_preloadFunc;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
