#include "typefolder.h"
#include "options.h"
#include "imagedb.h"
#include "contentfolder.h"
TypeFolder::TypeFolder( const QString& optionGroup, const ImageSearchInfo& info, Browser* parent )
    :Folder( info, parent ), _optionGroup ( optionGroup )
{
    setText( Options::instance()->textForOptionGroup( optionGroup ) );
    setPixmap( Options::instance()->iconForOptionGroup( _optionGroup ) );
}

FolderAction* TypeFolder::action( bool /* ctrlDown */ )
{
    return new TypeFolderAction( _optionGroup, _info, _browser );
}

TypeFolderAction::TypeFolderAction( const QString& optionGroup, const ImageSearchInfo& info,
                                    Browser* parent )
    :FolderAction( info, parent ), _optionGroup( optionGroup )
{
}

void TypeFolderAction::action()
{
    _browser->clear();

    QMap<QString, int> map = ImageDB::instance()->classify( _info, _optionGroup );
    for( QMapIterator<QString,int> it= map.begin(); it != map.end(); ++it ) {
        if ( it.key() != QString::fromLatin1( "__NONE__" ) ) {
            new ContentFolder( _optionGroup, it.key(), it.data(), _info, _browser );
        }
    }

    // Add the none option to the end
    int i = map[QString::fromLatin1("__NONE__")];
    if ( i != 0 )
        new ContentFolder( _optionGroup, QString::fromLatin1( "__NONE__" ), i, _info, _browser );
}

