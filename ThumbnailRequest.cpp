#include "ThumbnailRequest.h"
#include "ThumbnailView.h"
ThumbnailRequest::ThumbnailRequest()
    :ImageRequest()
{
}

ThumbnailRequest::ThumbnailRequest( const QString& fileName, const QSize& size, int angle, ThumbnailView* client)
    :ImageRequest( fileName, size, angle, client ), _thumbnailView( client ), _fileName( fileName )
{
}

bool ThumbnailRequest::stillNeeded() const
{
    return _thumbnailView->thumbnailStillNeeded( _fileName );
}
