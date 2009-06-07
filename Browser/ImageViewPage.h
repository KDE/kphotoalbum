#ifndef IMAGEVIEWPAGE_H
#define IMAGEVIEWPAGE_H
#include "BrowserPage.h"
#include <DB/ImageSearchInfo.h>

namespace Browser {

/**
 * \brief The page showing the actual images.
 *
 * See \ref Browser for a detailed description of how this fits in with the rest of the classes in this module
 */
class ImageViewPage :public BrowserPage
{
public:
    ImageViewPage( const DB::ImageSearchInfo& info, BrowserWidget* browser );
    ImageViewPage( const QString& context, BrowserWidget* browser );
    OVERRIDE void activate();
    OVERRIDE Viewer viewer();
    OVERRIDE bool isSearchable() const;
    OVERRIDE bool showDuringMovement() const;


private:
    QString _context;
};

}

#endif /* IMAGEVIEWPAGE_H */

