#ifndef THUMBNAILREQUEST_H
#define THUMBNAILREQUEST_H
#include "ImageManager/ImageRequest.h"

namespace ThumbnailView
{
class ThumbnailWidget;

class ThumbnailRequest :public ImageManager::ImageRequest
{
public:
    ThumbnailRequest();
    ThumbnailRequest( const QString& fileName, const QSize& size, int angle, ThumbnailWidget* client);
    virtual bool stillNeeded() const;

private:
    ThumbnailWidget* _thumbnailView;
    QString _fileName;
};

}

#endif /* THUMBNAILREQUEST_H */

