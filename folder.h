#ifndef FOLDER_H
#define FOLDER_H
#include <qiconview.h>
#include "browser.h"
#include <qstring.h>
#include "imagesearchinfo.h"

class FolderAction;

class Folder :public QIconViewItem {

public:
    Folder( const ImageSearchInfo& info, Browser* parent );
    virtual FolderAction* action( bool ctrlDown = false ) = 0;

protected:
    Browser* _browser;
    ImageSearchInfo _info;
};

class FolderAction
{
public:
    FolderAction( const ImageSearchInfo& info, Browser* browser );
    virtual ~FolderAction() {}
    virtual void action() = 0;
    virtual bool showsImages() = 0;

protected:
    Browser* _browser;
    ImageSearchInfo _info;
};

#endif /* FOLDER_H */

