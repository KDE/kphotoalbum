#include "thumbnailrequest.h"
ThumbnailRequest::ThumbnailRequest()
    :ImageRequest()
{
}

ThumbnailRequest::ThumbnailRequest( const QString& fileName, const QSize& size, int angle, ThumbNail* client)
    :ImageRequest( fileName, size, angle, client ), _thumbnail( client )
{
}

bool ThumbnailRequest::stillNeeded() const
{
    QIconView* view = _thumbnail->iconView();
    QRect content = QRect( view->contentsX(), view->contentsY(), view->visibleWidth(), view->visibleHeight() );
    bool visible = _thumbnail->rect().intersects( content );
    return visible;
}
