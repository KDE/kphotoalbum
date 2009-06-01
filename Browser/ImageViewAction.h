#ifndef IMAGEVIEWACTION_H
#define IMAGEVIEWACTION_H
#include "BrowserAction.h"
#include <DB/ImageSearchInfo.h>

namespace Browser {

class ImageViewAction :public BrowserAction
{
public:
    ImageViewAction( const DB::ImageSearchInfo& info, BrowserWidget* browser );
    ImageViewAction( const QString& context, BrowserWidget* browser );
    OVERRIDE void activate();
    OVERRIDE Viewer viewer();
    OVERRIDE bool isSearchable() const;


private:
    QString _context;
};

}

#endif /* IMAGEVIEWACTION_H */

