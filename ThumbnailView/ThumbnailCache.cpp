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
#include <QTimer>

#include "ThumbnailView/ThumbnailRequest.h"
#include "DB/ResultId.h"
#include "Settings/SettingsData.h"
#include "ImageManager/Manager.h"

/** Henners debug output. Will go away soon */
#undef HENNER_EXPERIMENTING

#include <stdlib.h>
#include <kdebug.h>

/** experiment with prewarming. This will soon be enabled by default */
static const char* environVar = "KPA_THUMBCACHE_WARMING";
static const char* prewarm = getenv(environVar);
static bool DO_PREWARM = (prewarm == NULL) || strcmp(prewarm, "1") == 0;

ThumbnailView::ThumbnailCache::ThumbnailCache()
    : _displayList(NULL), _hotFrom(0), _hotTo(0), _lastHotFrom(0) {
    _asyncWarmingTimer = new QTimer( this );
    _asyncWarmingTimer->setSingleShot(true);
    connect( _asyncWarmingTimer, SIGNAL( timeout() ),
             this,               SLOT( slotAsyncCacheWarming() ) );
#if 1
    if (DO_PREWARM) {
        kDebug() << "Do cache prewarming.; " << environVar << " not set or set to 1";
    } else {
        kDebug() << "Don't do cache prewarming. " << environVar << "=" << prewarm;
    }
#endif
}

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

void ThumbnailView::ThumbnailCache::setDisplayList(const DB::ConstResultPtr& list) {
    _displayList = list;
}

void ThumbnailView::ThumbnailCache::setThumbnailSize(const QSize& thumbSize) {
    if (thumbSize != _thumbSize) {
        _thumbSize = thumbSize;
        clear();
    }
}

// Warm the cache around the current hot area. This slot is called
// asynchronously after the user has stopped scrolling.
void ThumbnailView::ThumbnailCache::slotAsyncCacheWarming() {
    if (!DO_PREWARM || _displayList.isNull() || !_thumbSize.isValid())
        return;

    const bool scrollDown = _hotFrom > _lastHotFrom;
    _lastHotFrom = _hotFrom;

    const int page = _hotTo - _hotFrom;
    const int thumbSize = 4 * _thumbSize.width() * _thumbSize.height();
    const int totalCacheableThumbs = Settings::SettingsData::instance()->thumbnailCacheBytes() / thumbSize;
    // lets only use a part of the thumbnail cache because right now we're
    // using the global QPixmapCache that is used by others as well.
    const int preloadable = 3 * (totalCacheableThumbs - page) / 4;
#ifdef HENNER_EXPERIMENTING
    kDebug() << "range: " << _hotFrom << "-" << _hotTo << "; totalCacheable=" << totalCacheableThumbs << "; page=" << page << "; preloadable=" << preloadable;
#endif
    _requestedImagesLock.lock();
    _requestedImages.clear();
    _requestedImagesLock.unlock();
    ImageManager::Manager::instance()->stop(this);

    // First, we make sure that the closer areas are covered: next and
    // previous page.

    // If we don't have much preload spots to spend, favour the
    // direction in which we're going.
    const int likelyDir = ((preloadable < 2*page)
                           ? qMin(preloadable, page)
                           : page);
    const int unlikelyDir = ((preloadable < 2*page)
                             ? preloadable - likelyDir
                             : page);
    if (scrollDown) {
        requestRange(_hotTo, _hotTo + likelyDir, "next page");
        requestRange(_hotFrom - unlikelyDir, _hotFrom, "previous page");
    } else {
        requestRange(_hotFrom - likelyDir, _hotFrom, "previous page");
        requestRange(_hotTo, _hotTo + unlikelyDir, "next page");
    }

    // If we still have preload space to spend, load it now.
    if (preloadable > 2*page) {
        const int limit = preloadable / 2;
        requestRange(_hotTo + page, _hotTo + limit, "further next");
        requestRange(_hotFrom - limit, _hotFrom - page, "further prev");
    }
}

void ThumbnailView::ThumbnailCache::setHotArea(int from, int to) {
    const bool anyChange = (_hotFrom != from) || (_hotTo != to);
    _hotFrom = from;
    _hotTo = to;
    if (anyChange) {
#ifdef HENNER_EXPERIMENTING
        if (_asyncWarmingTimer->isActive())
            kDebug() << "WARMING PENDING ...";
#endif
        // Do the asynchronous warming after we've settled a bit.
        _asyncWarmingTimer->stop();
        _asyncWarmingTimer->start( 150 );
    }
}

void ThumbnailView::ThumbnailCache::requestRange(int from, int to, const char* purpose) {
    int debugCount = 0;
    ImageManager::Manager* imgManager = ImageManager::Manager::instance();
    if (from < 0) from = 0;
    if (to > _displayList->size()) to = _displayList->size();
    for (int i = from; i < to; ++i) {
        const DB::ResultId id = _displayList->at(i);
        if (QPixmapCache::find(thumbnailPixmapCacheKey(id)) != NULL)
            continue;
        DB::ImageInfoPtr info = id.fetchInfo();
        const QString fileName = info->fileName(DB::AbsolutePath);
        _requestedImagesLock.lock();
        _requestedImages.insert(fileName, id);
        _requestedImagesLock.unlock();
        ++debugCount;
        ImageManager::ImageRequest* request
            = new ThumbnailCacheRequest(fileName, _thumbSize,
                                        info->angle(), this);
        request->setPriority( ImageManager::ThumbnailInvisible );
        _requestedImagesLock.lock();
        _requestedImages.insert(fileName, id);
        _requestedImagesLock.unlock();
        imgManager->load( request );
    }
#ifdef HENNER_EXPERIMENTING
    kDebug() << "Request " << from << "-" << to << "; total=" << (to - from) << "; necessary=" << debugCount << "; " << purpose;
#else
    Q_UNUSED(purpose);
#endif
}

void ThumbnailView::ThumbnailCache::pixmapLoaded( const QString& fileName,
                                                  const QSize& size, const QSize& fullSize,
                                                  int angle, const QImage& image,
                                                  const bool loadedOK) {
    Q_UNUSED(fullSize);
    Q_UNUSED(angle);

    if (!loadedOK || image.isNull())
        return;
    QPixmap pixmap( size );
    pixmap = QPixmap::fromImage( image );

    QMutexLocker l(&_requestedImagesLock);
    RequestedMap::iterator found = _requestedImages.find(fileName);
    if (found != _requestedImages.end()) {
        insert(found.value(), pixmap);
        _requestedImages.erase(found);
#ifdef HENNER_EXPERIMENTING
        if (_requestedImages.size() == 0)
            kDebug() << "done.";
#endif
    }
#ifdef HENNER_EXPERIMENTING
    else {
        kDebug() << "throwing away result." << fileName;
    }
#endif
}

bool ThumbnailView::ThumbnailCache::thumbnailStillNeeded(const QString& fileName) const {
    QMutexLocker l(&_requestedImagesLock);
    bool needed = _requestedImages.contains(fileName);
#ifdef HENNER_EXPERIMENTING
    if (!needed) kDebug() << "don't need " << fileName << "anymore.";
#endif
    return needed;
}

QString ThumbnailView::ThumbnailCache::thumbnailPixmapCacheKey(const DB::ResultId& id) {
    return QString::fromLatin1("thumbnail:%1").arg(id.fileId());
}

#include "ThumbnailCache.moc"
