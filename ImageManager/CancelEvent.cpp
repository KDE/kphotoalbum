#include "CancelEvent.h"
#include "ImageRequest.h"
ImageManager::CancelEvent::CancelEvent( ImageRequest* request )
    : QEvent( static_cast<QEvent::Type>(CANCELEVENTID)), m_request( request )
{
}

ImageManager::CancelEvent::~CancelEvent()
{
    delete m_request;
}

ImageManager::ImageRequest * ImageManager::CancelEvent::request() const
{
    return m_request;
}
