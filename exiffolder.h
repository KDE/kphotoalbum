#ifndef EXIFFOLDER_H
#define EXIFFOLDER_H

#include "folder.h"

class ExifFolder : public Folder {

public:
    ExifFolder(  const ImageSearchInfo& info, Browser* browser );
    virtual FolderAction* action( bool ctrlDown = false );
    virtual QPixmap pixmap();
    virtual QString text() const;
    virtual QString countLabel() const;
};


#endif /* EXIFFOLDER_H */

