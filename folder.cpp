#include "folder.h"
#include <klocale.h>
#include "options.h"
#include "imagedb.h"
#include "undoredoobject.h"


Folder::Folder( const ImageSearchInfo& info, Browser* parent )
    :QIconViewItem( parent ), _browser( parent ), _info( info )
{
}


FolderAction::FolderAction( const ImageSearchInfo& info, Browser* browser )
    : _browser( browser ), _info( info )
{
}






