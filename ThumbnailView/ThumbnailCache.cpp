/* Copyright (C) 2008 Henner Zeller <h.zeller@acm.org>

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
#include "ThumbnailView/ThumbnailCache.h"

#include <QPixmapCache>

#include "DB/ResultId.h"

bool ThumbnailView::ThumbnailCache::find(const DB::ResultId& id,
                                         QPixmap *result) const {
    Q_ASSERT(result != NULL);
    return QPixmapCache::find( thumbnailPixmapCacheKey(id), *result );
}

void ThumbnailView::ThumbnailCache::insert(const DB::ResultId& id,
                                           const QPixmap &pixmap) {
    QPixmapCache::insert( thumbnailPixmapCacheKey(id), pixmap );
}

void ThumbnailView::ThumbnailCache::clear() {
    QPixmapCache::clear();
}

QString ThumbnailView::ThumbnailCache::thumbnailPixmapCacheKey(const DB::ResultId& id) {
    return QString::fromLatin1("thumbnail:%1").arg(id.fileId());
}
