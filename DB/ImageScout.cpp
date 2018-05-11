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

using namespace DB;

// 1048576 yields about 20% better performance than 65536 w/
// 3000 1M images on a Seagate Barracuda 2.5" 2TB disk, 5400 RPM
#define SCOUTBUFSIZE (1048576)

ImageScoutThread::ImageScoutThread( const ImageScoutList &list )
  : m_list(list),
    m_loadedCount(0)
{
}

void ImageScoutThread::incrementLoadedCount()
{
  m_loadedCount++;
}

void ImageScoutThread::run()
{
    int count = 0;
    for ( ImageScoutList::ConstIterator it = m_list.begin(); it != m_list.end(); ++it, ++count ) {
      // If we're behind the reader, move along
      if (m_loadedCount.load() >= count) {
          continue;
      // Don't get too far ahead of the loader, or we just waste memory
      } else {
          while (count >= m_loadedCount.load() + 20) {
              QThread::msleep(10);
          }
      }
        QFile file(( *it ).absolute());
        //      qDebug() << "Scout " << count << " " << ( *it ).absolute();
        if ( file.open(QIODevice::ReadOnly) ) {
            char tmp_buf[SCOUTBUFSIZE];
            QDataStream str( &file );
            while (! file.atEnd()) {
                if ( str.readRawData( tmp_buf, SCOUTBUFSIZE ) <= 0 || 
                     isInterruptionRequested() ||
                     m_loadedCount.load() >=count )
                    break;
            }
            file.close();
        }
        if ( isInterruptionRequested() ) {
          return;
        }
    }
}
// vi:expandtab:tabstop=4 shiftwidth=4:
