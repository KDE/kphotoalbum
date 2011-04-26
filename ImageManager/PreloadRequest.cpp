#include "PreloadRequest.h"
#include "ThumbnailCache.h"
#include <QDebug>

ImageManager::PreloadRequest::PreloadRequest(const QString& fileName, const QSize& size, int angle, ImageClient* client) :
    ImageRequest( fileName, size, angle, client )
{
}

bool ImageManager::PreloadRequest::stillNeeded() const
{
    return !ThumbnailCache::instance()->contains( fileName() );
}
