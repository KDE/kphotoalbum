#ifndef THUMBNAILREQUEST_H
#define THUMBNAILREQUEST_H
#include "thumbnail.h"
#include "imagerequest.h"
class ThumbNail;

class ThumbnailRequest :public ImageRequest
{
public:
    ThumbnailRequest();
    ThumbnailRequest( const QString& fileName, const QSize& size, int angle, ThumbNail* client);
    virtual bool stillNeeded() const;

private:
    ThumbNail* _thumbnail;
};

#endif /* THUMBNAILREQUEST_H */

