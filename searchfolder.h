#ifndef SEARCHFOLDER_H
#define SEARCHFOLDER_H
#include "folder.h"

class SearchFolder :public Folder {

public:
    SearchFolder( const ImageSearchInfo& info, Browser* browser );
    virtual FolderAction* action( bool ctrlDown = false );
    virtual QPixmap pixmap();
    virtual QString text() const;

};

#endif /* SEARCHFOLDER_H */

