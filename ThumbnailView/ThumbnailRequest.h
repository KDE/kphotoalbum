#ifndef THUMBNAILREQUEST_H
#define THUMBNAILREQUEST_H
#include "ImageManager/ImageRequest.h"

namespace ThumbnailView
{
class ThumbnailView;

class ThumbnailRequest :public ImageManager::ImageRequest
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

