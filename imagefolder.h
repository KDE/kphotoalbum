#ifndef IMAGEFOLDER_H
#define IMAGEFOLDER_H
#include "folder.h"

class ImageFolder :public Folder {
public:
    ImageFolder( const ImageSearchInfo& info, Browser* parent );
    ImageFolder( const ImageSearchInfo& info, int from, int to, Browser* parent );
    virtual FolderAction* action( bool ctrlDown = false );
private:
    int _from, _to;
};

class ImageFolderAction :public FolderAction
{
public:
    ImageFolderAction( const ImageSearchInfo& info, int from, int to, Browser* browser );
    virtual void action();
    virtual bool showsImages() { return true; }
private:
    int _from, _to;
    bool _addExtraToBrowser;
};

#endif /* IMAGEFOLDER_H */

