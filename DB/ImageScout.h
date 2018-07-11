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

#ifndef IMAGESCOUT_H
#define IMAGESCOUT_H
#include "DB/FileName.h"
#include <QQueue>
#include <QAtomicInt>
#include <QMutex>
#include <QList>

namespace DB
{

typedef QQueue<DB::FileName> ImageScoutQueue;
class ImageScoutThread;

/**
 * Scout thread for image loading: preload images from disk to have them in
 * RAM to mask I/O latency.
 */
class ImageScout {
public:
    // count is an atomic variable containing the number of images
    // that have been loaded thus far.  Used to prevent the scout from
    // getting too far ahead, if it's able to run faster than the
    // loader.  Which usually it can't; with hard disks on any halfway
    // decent system, the loader can run faster than the disk.
    ImageScout(ImageScoutQueue &, QAtomicInt &count, int threads = 1);
    ~ImageScout();
    // Specify scout buffer size.  May not be called after starting the scout.
    void setBufSize(int);
    int getBufSize();
    // Specify how far we're allowed to run ahead of the loader, in images.
    // May not be called after starting the scout.
    void setMaxSeekAhead(int);
    int getMaxSeekAhead();
    // Specify how many bytes we read before moving on.
    // May not be called after starting the scout.
    void setReadLimit(int);
    int getReadLimit();
    // Start the scout running
    void start();

private:
    QMutex m_mutex;
    QList<ImageScoutThread *> m_scoutList;
    QAtomicInt m_preloadedCount;
    QAtomicInt m_skippedCount;
    bool m_isStarted;
    int m_scoutBufSize;
    int m_maxSeekAhead;
    int m_readLimit;
};
}

#endif /* IMAGESCOUT_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
