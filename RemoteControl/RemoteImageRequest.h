#ifndef REMOTECONTROL_REMOTEIMAGEREQUEST_H
#define REMOTECONTROL_REMOTEIMAGEREQUEST_H

#include "ImageManager/ImageRequest.h"
#include "RemoteInterface.h"
#include "Types.h"

namespace RemoteControl {

class RemoteImageRequest : public ImageManager::ImageRequest
{
public:
    RemoteImageRequest(const DB::FileName& fileName, const QSize& size, int angle, ViewType type, RemoteInterface* client);
    virtual bool stillNeeded() const;
    ViewType type() const;

private:
    RemoteInterface* m_interface;
    ViewType m_type;
};

} // namespace RemoteControl

#endif // REMOTECONTROL_REMOTEIMAGEREQUEST_H
