/* Copyright (C) 2003-2004 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "datebartip.h"
#include "imagedaterange.h"
#include "datebar.h"
#include <qdatetime.h>

DateBarTip::DateBarTip( DateBar* bar )
    : QToolTip( bar ),_bar( bar )
{
}

void DateBarTip::maybeTip( const QPoint& p )
{
    if ( !_bar->barAreaGeometry().contains( p ) )
        return;

    ImageDateRange range = _bar->rangeAt( p );
    ImageCount count = _bar->_dates.count( range.start(), range.end().max().addSecs(-1) );

    QString cnt;
    if ( count._rangeMatch != 0 && _bar->includeFuzzyCounts())
        cnt = QObject::tr("%1 exact matches + %2 range matches = %3 images").arg( count._exact ).arg( count._rangeMatch ).arg( count._exact + count._rangeMatch );
    else
        cnt = QObject::tr("%1 images").arg( count._exact );

    QString res = QObject::tr("<qt><p><b>%1 to %2<b></p><p>%3</p></qt>").arg(range.start().toString()).arg(range.end().toString())
                  .arg(cnt);
    tip( _bar->barRect( p ), res );
}
