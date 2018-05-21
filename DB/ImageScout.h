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

/**
 * Scout thread for image loading: preload images from disk to have them in
 * RAM to mask I/O latency.
 */

namespace DB
{

typedef QQueue<DB::FileName> ImageScoutQueue;
class ImageScoutThread;

class ImageScout {
public:
    ImageScout(ImageScoutQueue &, QAtomicInt &count, int threads = 1);
    ~ImageScout();

private:
    QMutex m_mutex;
    QList<ImageScoutThread *> m_scoutList;
    QAtomicInt m_preloadedCount;
};
}

#endif /* IMAGESCOUT_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
