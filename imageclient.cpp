#include "imageclient.h"
#include "imagemanager.h"

ImageClient::~ImageClient()
{
    ImageManager::instance()->stop( this );
}
