#ifndef CONTENTFOLDER_H
#define CONTENTFOLDER_H
#include "folder.h"

class ContentFolder :public Folder {
public:
    ContentFolder( const QString& optionGroup, const QString& value, int count,
                   const ImageSearchInfo& info, Browser* parent );
    ContentFolder( const QString& optionGroup, int count,
                   const ImageSearchInfo& info, Browser* parent );
    virtual FolderAction* action( bool ctrlDown = false );

private:
    QString _optionGroup;
    QString _value;
};

class ContentFolderAction :public FolderAction {

public:
    ContentFolderAction( const QString& optionGroup, const QString& value,
                         const ImageSearchInfo& info, Browser* parent );
    virtual void action();
    virtual bool showsImages() { return false; }

private:
    QString _optionGroup;
    QString _value;
};

#endif /* CONTENTFOLDER_H */

