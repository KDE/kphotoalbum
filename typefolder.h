#ifndef TYPEFOLDER_H
#define TYPEFOLDER_H
#include "folder.h"

class TypeFolder :public Folder {
public:
    TypeFolder( const QString& optionGroup, const ImageSearchInfo& info, Browser* parent );
    virtual FolderAction* action( bool ctrlDown = false );
private:
    QString _optionGroup;
};

class TypeFolderAction :public FolderAction {

public:
    TypeFolderAction( const QString& optionGroup, const ImageSearchInfo& info, Browser* parent  );
    virtual void action();
    virtual bool showsImages() { return false; }

private:
    QString _optionGroup;
};



#endif /* TYPEFOLDER_H */

