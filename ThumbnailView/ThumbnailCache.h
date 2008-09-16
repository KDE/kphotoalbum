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

#include <QObject>
#include <QString>
#include <QSize>
#include <QMutex>
#include <QHash>

#include "DB/Result.h"
#include "ImageManager/ImageClient.h"

class QPixmap;
class QTimer;

namespace DB {
    class ResultId;
}
namespace ThumbnailView {

/**
 * A cache for thumbnails. Right now, this is only a thin wrapper around
 * QPixmapCache but it is extracted here to eventually allow more control
 * over what is cached and anticipate use.
 * Note, this is work in progress... (TODO(hzeller): update comment)
 */
class ThumbnailCache : public QObject, public ImageManager::ImageClient {
    Q_OBJECT;
public:
    ThumbnailCache();

    /**
     * Find the pixmap for the given ResultId. If found, return 'true' and
     * insert found pixmap into result. Result must not be NULL.
     */
    bool find(const DB::ResultId& id, QPixmap *result) const;

    /** insert the pixmap for the given Media ID */
    void insert(const DB::ResultId& id, const QPixmap& pix);

    /** clear the cache */
    void clear();

    /**
     * set the hot area of thumbnails currently displayed. The values
     * denote indices in the display list.
     * The Cache uses this to decide what images not to throw away and
     * what images to preload.
     */
    void setHotArea(int from, int to);

    /** Set information about the thumbnails to be displayed.
     * TODO: view and cache should share a model */
    void setDisplayList(const DB::ConstResultPtr& displayList);
    void setThumbnailSize(const QSize& thumbSize);

public:
    bool thumbnailStillNeeded(const QString& fileName) const;

protected:
    // ImageManager interface.
    virtual void pixmapLoaded( const QString& fileName,
                               const QSize& size, const QSize& fullSize,
                               int angle, const QImage& image,
                               const bool loadedOK);

protected slots:
    void slotAsyncCacheWarming();

private:
    static QString thumbnailPixmapCacheKey(const DB::ResultId& id);

    /**
     * warm the cache for the given range.
     * Including 'from', exlcuding 'to'.
     */
    void requestRange(int from, int to, const char* desc);

    DB::ConstResultPtr _displayList;
    QSize _thumbSize;
    int _hotFrom, _hotTo;
    int _lastHotFrom;
    QTimer* _asyncWarmingTimer;

    mutable QMutex _requestedImagesLock;
    typedef QHash<QString, DB::ResultId> RequestedMap;
    RequestedMap _requestedImages;
};
}

#endif /* THUMBNAILVIEW_THUMBNAILCACHE_H */
