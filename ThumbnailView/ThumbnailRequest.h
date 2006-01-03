#ifndef THUMBNAILREQUEST_H
#define THUMBNAILREQUEST_H
#include "imagerequest.h"

namespace ThumbnailView
{
class ThumbnailView;

class ThumbnailRequest :public ImageRequest
{
public:
    ThumbnailRequest();
    ThumbnailRequest( const QString& fileName, const QSize& size, int angle, ThumbnailView* client);
    virtual bool stillNeeded() const;

private:
    ThumbnailView* _thumbnailView;
    QString _fileName;
};

}

#endif /* THUMBNAILREQUEST_H */

