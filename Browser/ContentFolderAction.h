#ifndef BROWSER_CONTENTFOLDERACTION_H
#define BROWSER_CONTENTFOLDERACTION_H
#include "Folder.h"

namespace Browser
{
class ContentFolderAction :public FolderAction {

public:
    ContentFolderAction( const DB::ImageSearchInfo& info, BrowserWidget* parent );
    virtual void action( BrowserItemFactory* factory );
    virtual bool showsImages() const { return false; }
    virtual bool contentView() const { return false; }
    virtual bool allowSort() const;
    virtual QString title() const;
};

}

#endif /* BROWSER_CONTENTFOLDERACTION_H */

