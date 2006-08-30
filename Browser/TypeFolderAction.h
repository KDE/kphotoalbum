#ifndef BROWSER_TYPEFOLDERACTION_H
#define BROWSER_TYPEFOLDERACTION_H

#include "Folder.h"
#include "DB/Category.h"
#include <qmap.h>

namespace Browser
{
class BrowserItemFactory;
class BrowserItem;

class TypeFolderAction :public FolderAction {

public:
    TypeFolderAction( const DB::CategoryPtr& category, const DB::ImageSearchInfo& info, BrowserWidget* parent  );
    virtual void action( BrowserItemFactory* factory );
    virtual bool showsImages() const { return false; }
    virtual bool contentView() const;
    virtual QString title() const;
    DB::CategoryPtr category() const;
    virtual DB::Category::ViewType viewType() const;

protected:
    bool populateBrowserWithHierachy( DB::CategoryItem* parentCategoryItem, const QMap<QString, uint>& images,
                                      const QMap<QString, uint>& videos, BrowserItemFactory* factory, BrowserItem* parentBrowserItem );
    void populateBrowserWithoutHierachy( const QMap<QString, uint>& images,
                                         const QMap<QString, uint>& videos, BrowserItemFactory* factory );

private:
    const DB::CategoryPtr _category;
};

}


#endif /* BROWSER_TYPEFOLDERACTION_H */

