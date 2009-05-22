#ifndef IMAGEVIEWACTION_H
#define IMAGEVIEWACTION_H
#include "BrowserAction.h"
#include <DB/ImageSearchInfo.h>

namespace Browser {

class ImageViewAction :public BrowserAction
{
public:
    ImageViewAction( BrowserWidget* browser, const DB::ImageSearchInfo& info );
    OVERRIDE void activate();
    OVERRIDE Viewer viewer();

private:
    DB::ImageSearchInfo _info;
};

}

#endif /* IMAGEVIEWACTION_H */

