#ifndef SEARCHFOLDER_H
#define SEARCHFOLDER_H
#include "folder.h"

class SearchFolder :public Folder {

public:
    SearchFolder( const ImageSearchInfo& info, Browser* browser );
    virtual FolderAction* action( bool ctrlDown = false );

};

#endif /* SEARCHFOLDER_H */

