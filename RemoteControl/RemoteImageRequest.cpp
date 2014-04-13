#include "RemoteImageRequest.h"

namespace RemoteControl {

RemoteImageRequest::RemoteImageRequest(const DB::FileName& fileName, const QSize& size, int angle, RemoteInterface* client)
    : ImageManager::ImageRequest(fileName, size, angle,client), m_interface(client)
{
}

bool RemoteImageRequest::stillNeeded() const
{
    return m_interface->requestStillNeeded(fileSystemFileName());
}

} // namespace RemoteControl
