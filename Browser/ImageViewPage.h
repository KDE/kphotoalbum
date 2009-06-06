#ifndef IMAGEVIEWPAGE_H
#define IMAGEVIEWPAGE_H
#include "BrowserPage.h"
#include <DB/ImageSearchInfo.h>

namespace Browser {

class ImageViewPage :public BrowserPage
{
public:
    ImageViewPage( const DB::ImageSearchInfo& info, BrowserWidget* browser );
    ImageViewPage( const QString& context, BrowserWidget* browser );
    OVERRIDE void activate();
    OVERRIDE Viewer viewer();
    OVERRIDE bool isSearchable() const;


private:
    QString _context;
};

}

#endif /* IMAGEVIEWPAGE_H */

