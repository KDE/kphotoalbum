#include "ThumbnailRequest.h"
#include "ThumbnailView.h"

ThumbnailView::ThumbnailRequest::ThumbnailRequest()
    :ImageRequest()
{
}

ThumbnailView::ThumbnailRequest::ThumbnailRequest( const QString& fileName, const QSize& size, int angle, ThumbnailView* client)
    : ImageManager::ImageRequest( fileName, size, angle, client ), _thumbnailView( client ), _fileName( fileName )
{
}

bool ThumbnailView::ThumbnailRequest::stillNeeded() const
{
    return _thumbnailView->thumbnailStillNeeded( _fileName );
}
