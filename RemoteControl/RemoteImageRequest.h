#ifndef REMOTECONTROL_REMOTEIMAGEREQUEST_H
#define REMOTECONTROL_REMOTEIMAGEREQUEST_H

#include "ImageManager/ImageRequest.h"
#include "RemoteInterface.h"

namespace RemoteControl {

class RemoteImageRequest : public ImageManager::ImageRequest
{
public:
    RemoteImageRequest(const DB::FileName& fileName, const QSize& size, int angle, RemoteInterface* client);
    virtual bool stillNeeded() const;
private:
    RemoteInterface* m_interface;
};

} // namespace RemoteControl

#endif // REMOTECONTROL_REMOTEIMAGEREQUEST_H
