#include "RemoteImageRequest.h"

namespace RemoteControl {

RemoteImageRequest::RemoteImageRequest(const DB::FileName& fileName, const QSize& size, int angle, ViewType type, RemoteInterface* client)
    : ImageManager::ImageRequest(fileName, size, angle,client), m_interface(client), m_type(type)
{
}

bool RemoteImageRequest::stillNeeded() const
{
    return m_interface->requestStillNeeded(fileSystemFileName());
}

ViewType RemoteImageRequest::type() const
{
    return m_type;
}

} // namespace RemoteControl
