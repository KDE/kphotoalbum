#ifndef EXIFFOLDER_H
#define EXIFFOLDER_H

#include "Folder.h"

namespace Browser
{

class ExifFolder : public Folder {

public:
    ExifFolder(  const DB::ImageSearchInfo& info, BrowserWidget* browser );
    virtual FolderAction* action( bool ctrlDown = false );
    virtual QPixmap pixmap();
    virtual QString text() const;
    virtual QString imagesLabel() const;
    virtual QString moviesLabel() const;
};

}

#endif /* EXIFFOLDER_H */

