#ifndef IMAGEVIEWACTION_H
#define IMAGEVIEWACTION_H
#include "BrowserAction.h"
#include <DB/ImageSearchInfo.h>

namespace Browser {

class ImageViewAction :public BrowserAction
{
public:
    ImageViewAction( BrowserWidget* browser, const DB::ImageSearchInfo& info );
    virtual void activate();

private:
    DB::ImageSearchInfo _info;
};

}

#endif /* IMAGEVIEWACTION_H */

