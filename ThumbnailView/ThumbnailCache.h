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
#ifndef THUMBNAILVIEW_THUMBNAILCACHE_H
#define THUMBNAILVIEW_THUMBNAILCACHE_H

#include <QString>

class QPixmap;
namespace DB {
    class ResultId;
}
namespace ThumbnailView {

/**
 * A cache for thumbnails. Right now, this is only a thin wrapper around
 * QPixmapCache but it is extracted here to eventually allow more control
 * over what is cached and anticipate use.
 */
class ThumbnailCache {
public:
    /**
     * Find the pixmap for the given ResultId. If found, return 'true' and
     * insert found pixmap into result. Result must not be NULL.
     */
    bool find(const DB::ResultId& id, QPixmap *result) const;

    /** insert the pixmap for the given Media ID */
    void insert(const DB::ResultId& id, const QPixmap& pix);

    /** clear the cache */
    void clear();

private:
    static QString thumbnailPixmapCacheKey(const DB::ResultId& id);
};
}

#endif /* THUMBNAILVIEW_THUMBNAILCACHE_H */
