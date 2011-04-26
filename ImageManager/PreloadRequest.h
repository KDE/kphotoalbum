#ifndef VIDEOREQUEST_H
#define VIDEOREQUEST_H
#include "ImageRequest.h"

namespace ImageManager {

class PreloadRequest : public ImageRequest
{
public:
    explicit PreloadRequest( const QString& fileName, const QSize& size, int angle, ImageClient* client);
    OVERRIDE bool stillNeeded() const;
};

}

#endif // VIDEOREQUEST_H
